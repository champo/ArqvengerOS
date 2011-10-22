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

size_t _read(int fd, void* buf, size_t length) {

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->read == NULL) {
        return -1;
    }

    return fileDescriptor->ops->read(fileDescriptor, buf, length);
}

size_t _write(int fd, const void* buf, size_t length) {

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL || fileDescriptor->ops->write == NULL) {
        return -1;
    }

    return fileDescriptor->ops->write(fileDescriptor, buf, length);
}

int _open(const char* path, int flags, int mode) {
    struct fs_Inode* curdir;
    struct fs_Inode* next;
    struct fs_DirectoryEntry nextdir;
    int index = 0;
    char entry[ENTRY_NAME_MAX_LEN];
    int len = strlen(path);
    int i, gid, uid, file_gid, file_uid;
    int fd = -1;
    int flag = 0;
    int error = 0;

    struct Process* process = scheduler_current();

    for (i = 0; i < MAX_OPEN_FILES; i++) {
        if (process->fdTable[i].inode == NULL) {
            fd = i;
            i = MAX_OPEN_FILES;
        }
    }

    if (fd == -1) {
        return -1;
    }

    if (path[index] == '/') {
        curdir = fs_root();
        index++;
    } else {
        //TODO curdir = getcwd();
    }
    while (!flag && !error) {

        i = 0;

        while (path[index] != '/' && index != len) {
            entry[i] = path[index];
            index++;
            i++;
        }
        entry[i] = '\0';


        nextdir = fs_findentry(curdir, entry);

        if (nextdir.inode == 0) {
            error = 1;
        } else {
            next = fs_inode_open(nextdir.inode);
            fs_inode_close(curdir);
            curdir = next;
        }

        if ( index == len ) {
            flag = 1;
        } else {
            index++;
        }
    }

    if (!error) {
        //Hermoso caso en donde la entry existe
        file_uid = curdir->data->uid;
        file_gid = curdir->data->gid;
        gid = process->gid;
        uid = process->uid;

        unsigned short perms = curdir->data->typesAndPermissions & 0x0FFF;

        if (flags & O_RDONLY || flags & O_RDWR) {
            if (!(perms & S_IROTH) &&
                !(perms & S_IRGRP && file_gid == gid) &&
                !(perms & S_IRUSR && file_uid == uid)) {
                return -1;
            }
        }

        if (flags & O_WRONLY || flags & O_RDWR) {
            if(!(perms & S_IWOTH) &&
               !(perms & S_IWGRP && file_gid == gid) &&
               !(perms & S_IWUSR && file_uid == uid)) {
                return -1;
            }
        }

        //Pasamos los permisos
        process->fdTable[fd] = fs_fd(next, flags);
        return fd;
    }
    if (flag && (flags & O_CREAT)) {
        //Si el path existe, pero la entry no (flag && error) y en flags esta O_CREAT
        //trato de crear y abrir el archivo
        return _creat(path, mode);
    }
    return -1;
}

int _creat(const char* path, int mode) {
}

int _close(int fd) {

    struct Process* process = process_table_get(_getpid());
    struct FileDescriptor* fileDescriptor = &(process->fdTable[fd]);

    if (fileDescriptor->inode == NULL) {
        return -1;
    }
    if (fileDescriptor->ops->close != NULL) {
        fileDescriptor->ops->close(fileDescriptor);
    }
    fs_inode_close(fileDescriptor->inode);
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

