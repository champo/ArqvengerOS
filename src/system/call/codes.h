#ifndef     _system_call_codes_header_
#define     _system_call_codes_header_

#define     _SYS_READ       3
#define     _SYS_WRITE      4
#define     _SYS_IOCTL      54
#define     _SYS_OPEN       5
#define     _SYS_CLOSE      6
#define     _SYS_CREAT      8

#define     _SYS_TIME           13
#define     _SYS_TICKS          191

#define     _SYS_PINFO          999
#define     _SYS_SETPPERSONA    998

#define _SYS_EXIT 93
#define _SYS_YIELD 124
#define _SYS_GETPID 172
#define _SYS_GETPPID 173
#define _SYS_WAIT 1072
#define _SYS_KILL 129

#define _SYS_SLEEP 162
#define _SYS_NICE 62
#define _SYS_RENICE 662

#define _SYS_RUN 1

#define _SYS_MKDIR 39
#define _SYS_RMDIR 40
#define _SYS_UNLINK 10
#define _SYS_RENAME 38
#define _SYS_CHDIR 12
#define _SYS_READDIR 89
#define _SYS_GETCWD 183
#define _SYS_SYMLINK 83
#define _SYS_MKFIFO 0xFA

#endif
