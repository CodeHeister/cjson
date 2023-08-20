#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <inttypes.h>

	#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
	#define BYTE_TO_BINARY(byte)  \
	  ((byte) & 0x80 ? '1' : '0'), \
	  ((byte) & 0x40 ? '1' : '0'), \
	  ((byte) & 0x20 ? '1' : '0'), \
	  ((byte) & 0x10 ? '1' : '0'), \
	  ((byte) & 0x08 ? '1' : '0'), \
	  ((byte) & 0x04 ? '1' : '0'), \
	  ((byte) & 0x02 ? '1' : '0'), \
	  ((byte) & 0x01 ? '1' : '0') 

	extern void printBytesAsBin(char*, uint64_t);
	extern void *debug_malloc(size_t, char*, uint64_t);
	extern void debug_free(void*, char*, uint64_t);

	#ifdef MEM_DEBUG
		#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
		#define free(ptr) debug_free(ptr, __FILE__, __LINE__)
	#endif

#ifdef __cplusplus
}
#endif

#endif // DEBUG_H_
