#ifndef __SYSTEM_STAT__
#define __SYSTEM_STAT__

struct stat {
    size_t inode;
    int mode;
    int uid;
    int gid;
    size_t size;
    int type;
};

#endif
