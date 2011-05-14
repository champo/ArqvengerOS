#ifndef __stdarg_header__

#define __stdarg_header__

typedef char* va_list;

#define va_start( list, arg ) ((list) = (char*)(&(arg) + 1))
#define va_end( list )
#define va_arg( list, type ) ((list) += sizeof(type), *(type*)((list) - sizeof(type)))
#define va_copy( dest, src ) ((dest) = (src))

#endif
