#ifndef _system_call_header_
#define _system_call_header_

#include "type.h"
#include "system/pinfo.h"
#include "system/fs/direntry.h"

size_t _write(int fd, const void* buf, size_t length);

size_t _read(int fd, void* buf, size_t length);

int _ioctl(int fd, int cmd, void* argp);

time_t _time(time_t *tp);

pid_t _getpid(void);

pid_t _getppid(void);

pid_t _run(void(*EntryPoint)(char*), char* args, int active);

void _exit(void);

pid_t _wait(void);

int _pinfo(struct ProcessInfo* data, size_t size);

void _sleep(int seconds);

int _nice(int priority);

int _creat(const char* path, int mode);

int _close(int fd);

int _open(const char* filename, int flags, int mode);

int _mkdir(const char* path, int mode);

int _rmdir(const char* path);

int _unlink(const char* path);

int _rename(const char* from, const char* to);

int _readdir(int fd, struct fs_DirectoryEntry* entry, int hidden);

int _chdir(const char* path);

int _getcwd(char* path, size_t len);

void _setProcessPersona(int pid, int uid, int gid);

#endif
