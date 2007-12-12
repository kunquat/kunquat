

#ifndef K_XMEMORY_H
#define K_XMEMORY_H


#include <stdlib.h>


#define xalloc(type)       malloc(sizeof(type))

#define xnalloc(type, n)   malloc(sizeof(type) * (n))
#define xcalloc(type, n)   calloc((n), sizeof(type))

#define xfree(ptr)         free(ptr)


#endif // K_XMEMORY_H


