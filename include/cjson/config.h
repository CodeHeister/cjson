#ifndef CJSON_PRINT_CONF_H_
#define CJSON_PRINT_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stddef.h>

	#define PRINT_GAP "    "

	extern size_t print_offset; // offset counter for print function
	extern size_t default_hash_length;

#ifdef __cplusplus
}
#endif

#endif
