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

static struct fs_Inode* resolve_path(const char* path);

static char* path_directory(const char* path);

static char* path_file(const char* path);

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

    char* base = path_directory(path);
    kprintf("Trying to open %s", base);
    char* filename = path_file(path);
    kprintf(" %s\n", filename);

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
       if (!(perms & S_IROTH) &&
            !(perms & S_IRGRP && file_gid == gid) &&
            !(perms & S_IRUSR && file_uid == uid)) {
            fs_inode_close(file);
            return -1;
        }
    }

    if ((flags & 3) == O_WRONLY || (flags & 3) == O_RDWR) {
       if(!(perms & S_IWOTH) &&
          !(perms & S_IWGRP && file_gid == gid) &&
          !(perms & S_IWUSR && file_uid == uid)) {
            fs_inode_close(file);
            return -1;
        }
    }

    //Pasamos los permisos
    process->fdTable[fd] = fs_fd(file, flags);
    fs_inode_close(file);
    return fd;
}

int _creat(const char* path, int mode) {

    char* base = path_directory(path);
    char* filename = path_file(path);

    kprintf("Trying to create %s %s\n", base, filename);
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

int _close(int fd) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL) {
        return -1;
    }
    if (fileDescriptor->ops->close != NULL) {
        fileDescriptor->ops->close(fileDescriptor);
    }
    fs_inode_close(fileDescriptor->inode);
    process->fdTable[fd].inode = NULL;
    return 0;

}

int _ioctl(int fd, int cmd, void* argp) {

    struct Process* process = scheduler_current();
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->ioctl == NULL) {
        return -1;
    }

    return fileDescriptor->ops->ioctl(fileDescriptor, cmd, argp);
}

int _mkdir(const char* path, int mode) {
}

int _rmdir(const char* path) {
}

int _unlink(const char* path) {
}

int _rename(const char* from, const char* to) {
}

int _readdir(int fd, struct fs_DirectoryEntry* entry) {
}

int _chdir(const char* path) {
}

int _getcwd(char* path, size_t len) {
}

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
    }

    kprintf("Giving dir with inode %u\n", curdir->number);
    return curdir;
}

char* path_directory(const char* path) {

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
        result = kalloc(sizeof(char) * (lastSlash + 1));
        strncpy(result, path, lastSlash);
        result[lastSlash + 1] = 0;
        kprintf("Path %s, lastSlash %d len %u res %s\n", path, lastSlash, len, result);
    }

    return result;
}

char* path_file(const char* path) {

    size_t len = strlen(path);

    // If we have a trailing slash, we ignore it
    if (path[len - 1] == '/') {
        len--;
    }

    // Isolate the last element so that we can ignore it
    int lastSlash;
    for (lastSlash = len - 1; lastSlash >= 0 && path[lastSlash] != '/'; lastSlash--);

    char* result = kalloc(sizeof(char) * (len - lastSlash));
    strncpy(result, path + (lastSlash + 1), len - lastSlash - 1);
    result[len - lastSlash] = 0;

    return result;
}

