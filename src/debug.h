#ifndef __DEBUG__
#define __DEBUG__

#if 1
#define enter() kprintf("Entering %s:%u\n", __func__, __LINE__)
#define leave() kprintf("leaving %s:%u\n", __func__, __LINE__)

#define mem_check() { if (check_malloc() != NULL) kprintf("check_malloc at %s:%d", __func__, __LINE__); }
#else
#define enter()
#define leave()

#define mem_check() { if (check_malloc() != NULL) kprintf("check_malloc at %s:%d", __func__, __LINE__); }
#endif

#endif
