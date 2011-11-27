#include "system/call.h"
#include "drivers/keyboard.h"
#include "drivers/tty/tty.h"
#include "system/process/table.h"
#include "system/process/process.h"
#include "system/scheduler.h"
#include "system/fs/fs.h"
#include "system/fs/inode.h"
#include "system/fs/direntry.h"
#include "system/kprintf.h"
#include "library/string.h"
#include "constants.h"
#include "library/stdio.h"
#include "debug.h"

static struct fs_Inode* resolve_path(const char* path);

static struct fs_Inode* resolve_sym_link(struct fs_Inode* symlink);

static int open_sym_link(struct fs_Inode* symlink, int flags, int mode);

static int can_read(struct fs_Inode* inode);

static int can_write(struct fs_Inode* inode);

/**
 * Reads on the correct file.
 *
 * @param fd, the file descriptor to be read.
 * @param buf, this string will containg what it's read.
 * @param length, the amount of characters to be read.
 * @return the amount of characters read.
 */
size_t _read(int fd, void* buf, size_t length) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->read == NULL) {
        return -1;
    }

    if ((fileDescriptor->flags & 3) == O_WRONLY) {
        return -1;
    }

    return fileDescriptor->ops->read(fileDescriptor, buf, length);
}

/**
 * Writes on the correct file.
 *
 * @param fd, the file descritor to be written.
 * @param buf, the string to be written.
 * @param length, the amount of characters to be written.
 * @return the number of characters written.
 */
size_t _write(int fd, const void* buf, size_t length) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->write == NULL) {
        return -1;
    }

    if ((fileDescriptor->flags & 3) == O_RDONLY) {
        return -1;
    }

    return fileDescriptor->ops->write(fileDescriptor, buf, length);
}

/**
 * Opens a file.
 *
 * @param path, the string where the file is located.
 * @param flags, the access mode.
 * @param mode indicates the permissions of the new file. It is only used if O_CREAT is specified in flags.
 * @return the file descriptor if success, -1 if error.
 */
int _open(const char* path, int flags, int mode) {

    struct Process* process = scheduler_current();
    int fd = -1;

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {
        if (process->fdTable[i].inode == NULL) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        return -1;
    }

    if (mode == 0) {
        mode = 00666;
    }

    char* base = path_directory(path);
    char* filename = path_file(path);

    int purpose;

    struct fs_Inode* directory = resolve_path(base);
    if (directory == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }

    struct fs_DirectoryEntry fileEntry = fs_findentry(directory, filename);
    if (fileEntry.inode == 0) {

        if (!(flags & O_CREAT) || _creat(path, mode) != 0) {
            fs_inode_close(directory);

            kfree(base);
            kfree(filename);

            return -1;
        }

        fileEntry = fs_findentry(directory, filename);
        if (fileEntry.inode == 0) {
            fs_inode_close(directory);

            kfree(base);
            kfree(filename);

            return -1;
        }
    }

    fs_inode_close(directory);

    kfree(base);
    kfree(filename);

    struct fs_Inode* file = fs_inode_open(fileEntry.inode);
    int gid, uid, file_gid, file_uid;

    file_uid = file->data->uid;
    file_gid = file->data->gid;
    gid = process->gid;
    uid = process->uid;

    unsigned short perms = fs_permission(file);

    if ((flags & 3) == O_RDONLY || (flags & 3) == O_RDWR) {
        if  (can_read(file) != 0) {
            fs_inode_close(file);
            return -1;
        }
    }

    if ((flags & 3) == O_WRONLY || (flags & 3) == O_RDWR) {
        if (can_write(file) != 0) {
            fs_inode_close(file);
            return -1;
        }
    }

    //Pasamos los permisos

    if (INODE_TYPE(file->data) == INODE_LINK) {
        fd = open_sym_link(file, flags, mode);
        fs_inode_close(file);
    } else {
        fs_fd(&process->fdTable[fd], file, flags);
        if (flags & O_APPEND) {
            process->fdTable[fd].offset = file->data->size;
        }
        fs_inode_close(file);
    }
    return fd;
}

/**
 * Creates a new file.
 *
 * @param path, the path of the new file.
 * @param, the permissions for the new file.
 * @return 0 if succes, -1 if error.
 */
int _creat(const char* path, int mode) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    struct fs_Inode* directory = resolve_path(base);
    if (directory == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }

    if (can_write(directory) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(directory);
        return -1;
    }

    if (fs_mknod(directory, filename, INODE_FILE) != 0) {
        kfree(base);
        kfree(filename);

        fs_inode_close(directory);
        return -1;
    }

    struct fs_DirectoryEntry fileEntry = fs_findentry(directory, filename);

    fs_inode_close(directory);
    kfree(base);
    kfree(filename);

    if (fileEntry.inode == 0) {
        return -1;
    }

    struct fs_Inode* file = fs_inode_open(fileEntry.inode);
    if (fs_set_permission(file, mode) != 0) {
        fs_inode_close(file);
        return -1;
    }
    fs_inode_close(file);

    return 0;

}

/**
 * Closes a file already opened by the process.
 *
 * @param fd, the file descriptor to be closed.
 * @return 0 if success, -1 if error.
 */
int _close(int fd) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL) {
        return -1;
    }

    return fs_fd_close(fileDescriptor);
}

/**
 * Does driver dependent operations.
 *
 * @param fd, the file descriptor of the file to be manipulated.
 * @param cmd, the command to be executed.
 * @param argp, the arguments of the command.
 */
int _ioctl(int fd, int cmd, void* argp) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->ioctl == NULL) {
        return -1;
    }

    return fileDescriptor->ops->ioctl(fileDescriptor, cmd, argp);
}

/**
 * Creates a new directory.
 *
 * @param path, the path of the directory to be created.
 * @param mode, the permissions of the directory.
 * @return 0 if success, other if error.
 */
int _mkdir(const char* path, int mode) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    struct fs_Inode* parent = resolve_path(base);
    if (parent == NULL) {
        kfree(base);
        kfree(filename);

        return -1;
    }

    if (can_write(parent) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(parent);
        return -1;
    }

    if (INODE_TYPE(parent->data) != INODE_DIR) {
        kfree(base);
        kfree(filename);
        fs_inode_close(parent);

        return -1;
    }

    int res = fs_mkdir(parent, filename);
    if (res != 0) {
        fs_inode_close(parent);
        kfree(base);
        kfree(filename);

        return res;
    }

    fs_inode_close(parent);
    kfree(base);
    kfree(filename);

    struct fs_Inode* directory = resolve_path(path);
    if (directory == NULL) {
        return -1;
    }

    res = fs_set_permission(directory, mode);
    fs_inode_close(directory);

    return res;
}

/**
 * Removes a directory if it is empty.
 *
 * @param path, the path of the directory to be erased.
 * @return 0 if success, other if error.
 */
int _rmdir(const char* path) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    struct fs_Inode* parent = resolve_path(base);
    if (parent == NULL) {
        kfree(base);
        kfree(filename);

        return -1;
    }

    if (can_write(parent) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(parent);
        return -1;
    }

    if (INODE_TYPE(parent->data) != INODE_DIR) {
        kfree(base);
        kfree(filename);
        fs_inode_close(parent);

        return -1;
    }

    struct fs_DirectoryEntry nextdir = fs_findentry(parent, filename);

    if (nextdir.inode == 0) {
        fs_inode_close(parent);
        return -1;
    }

    struct fs_Inode* curdir = fs_inode_open(nextdir.inode);

    if (can_write(curdir) != 0) {
        fs_inode_close(parent);
        fs_inode_close(curdir);
        return -1;
    }

    fs_inode_close(curdir);

    int res = fs_rmdir(parent, filename);

    kfree(base);
    kfree(filename);
    fs_inode_close(parent);

    return res;
}

/**
 * Remove a hard link from the file
 *
 * @param path, the path of the file to be unlinked.
 * @return 0 if success, other if error
 */
int _unlink(const char* path) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    struct fs_Inode* parent = resolve_path(base);
    if (parent == NULL) {
        kfree(base);
        kfree(filename);

        return -1;
    }

    if (can_write(parent) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(parent);
        return -1;
    }

    struct fs_DirectoryEntry nextdir = fs_findentry(parent, filename);

    if (nextdir.inode == 0) {
        fs_inode_close(parent);
        return -1;
    }


    struct fs_Inode* file = fs_inode_open(nextdir.inode);

    if (can_write(file) != 0) {
        fs_inode_close(parent);
        fs_inode_close(file);
        return -1;
    }

    fs_inode_close(file);

    int res = fs_unlink(parent, filename);

    kfree(base);
    kfree(filename);
    fs_inode_close(parent);

    return res;
}

/**
 * Renames a file.
 * @param from, the original path of the file.
 * @param to, the new path of the file.
 * @return 0 if success, other if error.
 */
int _rename(const char* source, const char* dest) {

    char* pathsource = path_directory(source);

    struct fs_Inode* sourcedir = resolve_path(pathsource);

    kfree(pathsource);
    
    //If the source directory doesn't exist, we return
    if (sourcedir == NULL) {
        return -1;
    }

    char* pathdest = path_directory(dest);

    struct fs_Inode* destdir = resolve_path(pathdest);

    kfree(pathdest);

    //If the destiny directory doesn't exist, we return
    if (destdir == NULL) {
        fs_inode_close(sourcedir);
        return -1;
    }

    char* filesource = path_file(source);

    struct fs_DirectoryEntry sourceEntry = fs_findentry(sourcedir, filesource);

    //If the inode specified for source, doesn't exist, we return
    if (sourceEntry.inode == 0) {
        fs_inode_close(sourcedir);
        fs_inode_close(destdir);
        kfree(filesource);
        return -1;
    }

    struct fs_Inode* sourceInode = fs_inode_open(sourceEntry.inode);
    
    //If for some reason we couldn't open the sorce inode, we return
    if (sourceInode == NULL) {
        fs_inode_close(sourcedir);
        fs_inode_close(destdir);
        kfree(filesource);
        return -1;
    }

    int sourceIsDir = 0;

    if (INODE_TYPE(sourceInode->data) == INODE_DIR) {
        sourceIsDir = 1;
    }


    //We check if we can actually delete the current source
    if (can_write(sourceInode) != 0) {
        kfree(filesource);
        fs_inode_close(sourcedir);
        fs_inode_close(destdir);
        fs_inode_close(sourceInode);
        return -1;
    }

    fs_inode_close(sourceInode);

    char* filedest = path_file(dest);
   
    //Let's check, if we want the file in the same place as before, it is an error.
    if (strcmp(filesource, filedest) == 0 && sourcedir->number == destdir->number) {
        fs_inode_close(sourcedir);
        fs_inode_close(destdir);
        kfree(filedest);
        kfree(filesource);
        return -1;
    }
    
    struct fs_DirectoryEntry destEntry = fs_findentry(destdir, filedest);

    //If the destiny inode already exists....
    if (destEntry.inode != 0) {
        struct fs_Inode* destInode = fs_inode_open(destEntry.inode);
        //Well, if it is not a directory....
        if (INODE_TYPE(destInode->data) != INODE_DIR) {
            //We check if the source is a directory. We cannot overwrite a file with a directory...
            if (sourceIsDir) {
                fs_inode_close(destInode);
                fs_inode_close(sourcedir);
                fs_inode_close(destdir);
                kfree(filedest);
                kfree(filesource);
                return -1;
            } else {
                //But if it is a file, we make it go away
                fs_inode_close(destInode);
                if (_unlink(dest) != 0) {
                    fs_inode_close(sourcedir);
                    fs_inode_close(destdir);
                    kfree(filedest);
                    kfree(filesource);
                    return -1;
                }
            }
        } else {
            //But if it's not, we must change the destination since we will write inside this directory!
            //Also, the name of the file will stay the same, but inside the new folder...
            fs_inode_close(destInode);
            char* stringDirDest = path_directory(dest);
            char* newDirDest = join_paths(stringDirDest, filedest);
            kfree(stringDirDest);
            destdir = resolve_path(newDirDest);
            char* stringFileError = join_paths(newDirDest, filesource);
            kfree(filedest);
            kfree(newDirDest);
            filedest = path_file(source);
            
            //Okay, so right here we are in a special hell where we must copy a file inside a directory
            //but what if a file with the same name exists in the directory?
            //well, if it exists almost always fails
            //It doesn't fail if both source and destiny are regular files, in which case it makes
            //a regular mv.
            
            struct fs_DirectoryEntry errorEntry = fs_findentry(destdir, filedest);

            //If the inode specified, exists...
            if (errorEntry.inode != 0) {
                struct fs_Inode* errorInode = fs_inode_open(errorEntry.inode);

                //For some reason we couldn't open the inode, we return
                if (errorInode == NULL) {
                    fs_inode_close(sourcedir);
                    fs_inode_close(destdir);
                    kfree(filedest);
                    kfree(filesource);
                    kfree(stringFileError);
                    return -1;
                }

                if (!(INODE_TYPE(errorInode->data) != INODE_DIR && !sourceIsDir)) {
                    fs_inode_close(sourcedir);
                    fs_inode_close(destdir);
                    fs_inode_close(errorInode);
                    kfree(filedest);
                    kfree(filesource);
                    kfree(stringFileError);
                    return -1;
                }

                fs_inode_close(errorInode);

                if (_unlink(stringFileError) != 0) {
                    kprintf("!%s!", stringFileError);
                    fs_inode_close(sourcedir);
                    fs_inode_close(destdir);
                    kfree(filedest);
                    kfree(stringFileError);
                    kfree(filesource);
                    return -1;
                }
            }
            kfree(stringFileError);
        }
    }
    
    //If we don't have the permissions to write in the destiny directory, we return
    if (can_write(destdir) != 0) {
        fs_inode_close(sourcedir);
        fs_inode_close(destdir);
        kfree(filesource);
        kfree(filedest);
        return -1;
    }

    //We actually move the thing and we return
    int ans = fs_rename(sourcedir, filesource, destdir, filedest);

    kfree(filesource);
    kfree(filedest);
    fs_inode_close(sourcedir);
    fs_inode_close(destdir);

    return ans;
}

/**
 * Reads an entry from a directory.
 *
 * @param fd, the file descriptor of the directory to be read.
 * @param entry, the output of the entry to be read.
 * @param hidden, should value 1 if hidden files are also wanted.
 * @return, 1 if something was read, 0 if not.
 */
int _readdir(int fd, struct fs_DirectoryEntry* entry, int hidden) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* des = &(process->fdTable[fd]);

    if (des->inode == NULL || des->ops->readdir == NULL) {
        return -1;
    }

    struct fs_DirectoryEntry res = des->ops->readdir(des);

    if (!hidden) {
        while (res.inode != 0 && res.name[0] == '.') {
            res = des->ops->readdir(des);
        }
    }

    if (res.inode == 0) {
        return 0;
    } else {
        *entry = res;
        return 1;
    }
}

/**
 * Changes the cwd.
 *
 * @param path, the path of the new cwd.
 * @return 0 if success, other if error.
 */
int _chdir(const char* path) {

    struct Process* process = scheduler_current();

    char* nwd = join_paths(process->cwd, path);

    struct fs_Inode* destination = resolve_path(nwd);

    if (destination == NULL) {
        kfree(nwd);
        return -1;
    }

    if (INODE_TYPE(destination->data) != INODE_DIR) {
        kfree(nwd);
        fs_inode_close(destination);
        return -1;
    }


    kfree(process->cwd);
    process->cwd = nwd;
    fs_inode_close(destination);

    return 0;
}


/**
 * Get the cwd.
 *
 * @param path, the string which will contain the cwd.
 * @param len, the maximum lenght of the string accepted.
 * @return 0 if success, other if error.
 */
int _getcwd(char* path, size_t len) {

    strncpy(path, scheduler_current()->cwd, len);
    path[len - 1] = 0;

    return 0;
}

/**
 * Goes through a path, resolving links and returning its inode.
 *
 * @param path, the path to be resolved.
 * @param purpose, 0 if the operation is a read, 1 if its a write, 2 if it can be both.
 * @returns, the inode referenced by path. NULL if error.
 */
struct fs_Inode* resolve_path(const char* path) {

    size_t len = strlen(path);
    size_t index = 0;
    char entry[ENTRY_NAME_MAX_LEN];
    struct fs_Inode* curdir;
    int permission = -1;

    if (path[index] == '/') {
        curdir = fs_root();
        index++;
    } else {
        curdir = resolve_path(scheduler_current()->cwd);
        if (curdir == NULL) {
            return NULL;
        }
    }

    permission = can_read(curdir);

    if (permission != 0) {
        fs_inode_close(curdir);
        return NULL;
    }

    while (index < len) {

        int i;
        for (i = 0; index < len && path[index] != '/'; i++, index++) {
            entry[i] = path[index];
        }
        index++;
        entry[i] = '\0';


        struct fs_DirectoryEntry nextdir = fs_findentry(curdir, entry);
        fs_inode_close(curdir);

        if (nextdir.inode == 0) {
            return NULL;
        }

        curdir = fs_inode_open(nextdir.inode);

        permission = can_read(curdir);

        if (permission != 0) {
            fs_inode_close(curdir);
            return NULL;
        }

        if (INODE_TYPE(curdir->data) == INODE_LINK) {
            struct fs_Inode* auxdir = resolve_sym_link(curdir);
            fs_inode_close(curdir);
            curdir = auxdir;
            if (curdir == NULL) {
                return NULL;
            }
        }
    }

    return curdir;
}

/**
 * Checks if the user can read an inode, checking permissions.
 *
 * @param inode, the inode to be read.
 * @return 0 if it is available, -1 if not.
 */
int can_read(struct fs_Inode* inode) {
    int gid, uid, file_gid, file_uid;

    struct Process* process = scheduler_current();

    gid = process->gid;
    uid = process->uid;

    if (uid == 0) {
        return 0;
    }

    file_uid = inode->data->uid;
    file_gid = inode->data->gid;

    unsigned short perms = fs_permission(inode);

    if (!(perms & S_IROTH) &&
        !(perms & S_IRGRP && file_gid == gid) &&
        !(perms & S_IRUSR && file_uid == uid)) {
        return -1;
    }

    return 0;
}

/**
 * Checks if the user can write an inode, checking permissions.
 *
 * @param inode, the inode to be written.
 * @return 0 if it is available, -1 if not.
 */
int can_write(struct fs_Inode* inode) {
    int gid, uid, file_gid, file_uid;

    struct Process* process = scheduler_current();

    gid = process->gid;
    uid = process->uid;

    if (uid == 0) {
        return 0;
    }

    file_uid = inode->data->uid;
    file_gid = inode->data->gid;

    unsigned short perms = fs_permission(inode);

    if (!(perms & S_IWOTH) &&
        !(perms & S_IWGRP && file_gid == gid) &&
        !(perms & S_IWUSR && file_uid == uid)) {
        return -1;
    }

    return 0;
}

/**
 * Resolves the real link to which a symbolic link is pointing.
 *
 * @param symlink, the inode of the symbolic link.
 * @param purpose, 0 if the operation is a read, 1 if it is a write, 2 if it can be both.
 * @return the inode to which the symbolic link references.
 */
struct fs_Inode* resolve_sym_link(struct fs_Inode* symlink) {

    int size = fs_get_inode_size(symlink);
    char* buff = kalloc(size + 1);

    buff = fs_symlink_read(symlink, size, buff);

    if (buff == NULL) {
        return NULL;
    }

    struct fs_Inode* ans = resolve_path(buff);
    kfree(buff);
    return ans;
}

/**
 * Opens a file that is really a symbolic link.
 *
 * @param symlink, the symbolic link to be opened.
 * @param flags, the access mode.
 * @param mode, mode indicates the permissions of the new file. It is only used if O_CREAT is specified in flags.
 * @return the file descriptor if success, -1 if error.
 */
int open_sym_link(struct fs_Inode* symlink, int flags, int mode) {

    int size = fs_get_inode_size(symlink);
    char* buff = kalloc(size + 1);

    buff = fs_symlink_read(symlink, size, buff);
    if (buff == NULL) {
        return -1;
    }
    int ans = _open(buff, flags, mode);
    kfree(buff);
    return ans;
}

/**
 * Create a symbolic link.
 *
 * @param path, the symbolic link to be created.
 * @param target, the target which will be referenced by path.
 * @return 0 if success, other if error.
 */
int _symlink(const char* path, const char* target) {

    char* base = path_directory(target);
    char* filename = path_file(target);

    struct fs_Inode* directory = resolve_path(base);
    if (directory == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }
    struct fs_DirectoryEntry fileEntry = fs_findentry(directory, filename);

    kfree(base);
    kfree(filename);
    fs_inode_close(directory);

    if (fileEntry.inode == 0) {
        return -1;
    }

    char* fulltarget = join_paths(scheduler_current()->cwd, target);

    base = path_directory(path);
    filename = path_file(path);

    directory = resolve_path(base);

    if (directory == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }

    if (can_write(directory) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(directory);
        return -1;
    }


    int ans = fs_symlink(directory, filename, fulltarget);
    kfree(base);
    kfree(filename);
    fs_inode_close(directory);

    return ans;
}

/**
 * Create a named pipe.
 *
 * @param path, the path of the pipe to be created.
 * @return 0 if success, other if error
 */
int _mkfifo(const char* path) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    struct fs_Inode* directory = resolve_path(base);
    if (directory == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }


    if (can_write(directory) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(directory);
        return -1;
    }


    int res = fs_mknod(directory, filename, INODE_FIFO);

    kfree(base);
    kfree(filename);

    fs_inode_close(directory);
    return res;
}

/**
 * Changes the permissions of a file.
 *
 * @param mode, the new permissions.
 * @param file, the path of the file.
 * @return 0 if success, -1 if error.
 */
int _chmod(int mode, const char* file) {

    char* base = path_directory(file);
    char* filename = path_file(file);

    struct fs_Inode* inode = resolve_path(base);
    if (inode == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }

    if (can_write(inode) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(inode);
        return -1;
    }


    struct fs_DirectoryEntry nextdir = fs_findentry(inode, file);
    fs_inode_close(inode);

    if (nextdir.inode == 0) {
        return NULL;
    }

    inode = fs_inode_open(nextdir.inode);

    if (can_write(inode) != 0) {
        fs_inode_close(inode);
        return -1;
    }

    int ans = fs_set_permission(inode, mode);

    fs_inode_close(inode);

    return ans;
}

int _stat(const char* entry, struct stat* data) {

    char* path = path_directory(entry);
    char* filename = path_file(entry);

    struct fs_Inode* directory = resolve_path(path);
    if (directory == NULL) {
        return -1;
    }

    struct fs_DirectoryEntry fileEntry = fs_findentry(directory, filename);

    kfree(path);
    kfree(filename);
    fs_inode_close(directory);

    if (fileEntry.inode == 0) {
        return -1;
    }

    struct fs_Inode* inode = fs_inode_open(fileEntry.inode);
    if (inode == NULL) {
        return -1;
    }

    data->inode = inode->number;
    data->size = inode->data->size;
    data->mode = fs_permission(inode);
    data->uid = inode->data->uid;
    data->gid = inode->data->gid;
    data->type = INODE_TYPE(inode->data);
    fs_inode_close(inode);

    return 0;
}

/**
 * Changes the owner and group of a file.
 *
 * @param file, the path of the file.
 * @return 0 if success, -1 if error.
 */
int _chown(const char* file) {

    char* base = path_directory(file);
    char* filename = path_file(file);

    struct fs_Inode* inode = resolve_path(base);
    if (inode == NULL) {
        kfree(base);
        kfree(filename);
        return -1;
    }

    if (can_write(inode) != 0) {
        kfree(base);
        kfree(filename);
        fs_inode_close(inode);
        return -1;
    }
    struct fs_DirectoryEntry nextdir = fs_findentry(inode, file);
    fs_inode_close(inode);

    if (nextdir.inode == 0) {
        return NULL;
    }

    inode = fs_inode_open(nextdir.inode);

    if (can_write(inode) != 0) {
        fs_inode_close(inode);
        return -1;
    }

    struct Process* process = scheduler_current();

    int ans = fs_set_own(inode, process->uid, process->gid);

    fs_inode_close(inode);

    return ans;
}

void _loglevel(int level) {
    set_log_level(level);
}
