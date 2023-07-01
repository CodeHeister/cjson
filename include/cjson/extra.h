#ifndef CJSON_EXTRA_H_
#define CJSON_EXTRA_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stddef.h>
	#include <limits.h>

	#define HASH_LIMIT UINT_MAX

	extern size_t rand_pos();
	extern char *clone2str(const char *);

#ifdef __cplusplus
}
#endif

#endif // CJSON_EXTRA_H_
