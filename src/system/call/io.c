#include "system/call.h"
#include "drivers/keyboard.h"
#include "drivers/tty/tty.h"
#include "system/process/table.h"
#include "system/process/process.h"
#include "system/scheduler.h"
#include "system/fs/fs.h"
#include "system/fs/inode.h"
#include "system/fs/direntry.h"
#include "library/string.h"
#include "constants.h"
#include "library/stdio.h"
#include "debug.h"

static struct fs_Inode* resolve_path(const char* path);

static struct fs_Inode* resolve_sym_link(struct fs_Inode* symlink);

static int open_sym_link(struct fs_Inode* symlink, int flags, int mode);

static char* path_directory(const char* path);

static char* path_file(const char* path);

static char* join_paths(const char* cwd, const char* path);

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

    if (flags & O_TRUNC) {
        _unlink(path);
        _creat(path, mode);
    }

    char* base = path_directory(path);
    char* filename = path_file(path);

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
        process->fdTable[fd] = fs_fd(file, flags);
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
int _rename(const char* from, const char* to) {
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
 * Joins the cwd and a relative path, resolving problems such as /.. and /. It takes in account full paths too.
 * 
 * @param cwd, the cwd.
 * @param path, a relative path.
 * @return, the complete path.
 */
char* join_paths(const char* cwd, const char* path) {

    size_t pathLen = strlen(path);
    char* nwd;

    if (path[0] == '/') {
        // If the new path is absolute little work needs to be done
        nwd = kalloc(sizeof(char) * (pathLen + 1));
        strcpy(nwd, path);

        return nwd;
    }

    size_t cwdLen = strlen(cwd);

    nwd = kalloc(sizeof(char) * (cwdLen + pathLen + 2));
    strcpy(nwd, cwd);

    int index = cwdLen;
    if (nwd[cwdLen - 1] == '/') {
        index--;
    }

    for (size_t pathIndex = 0; pathIndex < pathLen;) {

        size_t start = pathIndex;
        size_t componentLen = 0;

        for (; pathIndex < pathLen && path[pathIndex] != '/'; pathIndex++, componentLen++);
        // Make sure we skip any forward slashes
        pathIndex++;

        if (path[start] == '.') {

            if (componentLen == 1) {
                continue;
            } else if (componentLen == 2 && path[start + 1] == '.') {
                // Remove the last component of nwd
                for (; index > 0 && nwd[index] != '/'; index--);
                continue;
            }

        }

        nwd[index++] = '/';
        for (size_t i = 0; i < componentLen; i++, index++) {
            nwd[index] = path[start + i];
        }
    }

    if (index == 0) {
        nwd[index++] = '/';
    }

    nwd[index] = 0;

    return nwd;
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
 * @returns, the inode referenced by path. NULL if error.
 */
struct fs_Inode* resolve_path(const char* path) {

    size_t len = strlen(path);
    size_t index = 0;
    char entry[ENTRY_NAME_MAX_LEN];
    struct fs_Inode* curdir;

    if (path[index] == '/') {
        curdir = fs_root();
        index++;
    } else {
        curdir = resolve_path(scheduler_current()->cwd);
        if (curdir == NULL) {
            return NULL;
        }
    }

    if (can_read(curdir) != 0) {
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
        
        if (can_read(curdir) != 0) {
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
 * Separetes the directory in a path.
 *
 * @param path, the path to be analyzed.
 * @return the directory part of path.
 */
char* path_directory(const char* path) {

    if (strcmp(path, "/") == 0) {
        char* result = kalloc(sizeof(char) * 2);
        result[0] = '/';
        result[1] = '\0';
        return result;
    }

    size_t len = strlen(path);

    // If we have a trailing slash, we ignore it
    if (path[len - 1] == '/') {
        len--;
    }

    // Isolate the last element so that we can ignore it
    int lastSlash;
    for (lastSlash = len - 1; lastSlash >= 0 && path[lastSlash] != '/'; lastSlash--);

    char* result;
    if (lastSlash == -1) {
        // There is no directory component in this path
        result = kalloc(sizeof(char) * 2);
        strcpy(result, ".");
    } else if (lastSlash == 0) {
        // The directory component is root
        result = kalloc(sizeof(char) * 2);
        strcpy(result, "/");
    } else {
        result = kalloc(sizeof(char) * (lastSlash + 2));
        strncpy(result, path, lastSlash);
        result[lastSlash + 1] = 0;
    }

    return result;
}

/**
 * Isolates the file from the path.
 *
 * @param path, the path to be analyzed.
 * @return the file part of the path.
 */
char* path_file(const char* path) {

    if (strcmp(path, "/") == 0) {
        char* result = kalloc(sizeof(char) * 2);
        result[0] = '.';
        result[1] = '\0';
        return result;
    }

    size_t len = strlen(path);

    // If we have a trailing slash, we ignore it
    if (path[len - 1] == '/') {
        len--;
    }

    // Isolate the last element so that we can ignore it
    int lastSlash;
    for (lastSlash = len - 1; lastSlash >= 0 && path[lastSlash] != '/'; lastSlash--);

    char* result = kalloc(sizeof(char) * (len - lastSlash + 1));
    strncpy(result, path + (lastSlash + 1), len - lastSlash - 1);
    result[len - lastSlash] = 0;

    return result;
}

/**
 * Resolves the real link to which a symbolic link is pointing.
 * 
 * @param symlink, the inode of the symbolic link.
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
    
    struct fs_Inode* inode = resolve_path(file);
    if (inode == NULL) {
        return -1;
    }

    if (can_write(inode) != 0) {
        return -1;
    }
    
    return fs_set_permission(inode, mode);
}
