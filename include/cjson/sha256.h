#ifndef SHA256_H_
#define SHA256_H_

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdint.h>
	#include <stddef.h>

	typedef uint32_t hash_t;

	extern hash_t *bytes2sha256(const char *, size_t);
	extern hash_t *str2sha256(const char *);

	extern char *sha2562str(const hash_t *);

	extern int sha256Compare(const hash_t *, const hash_t *);

	extern int sha256Print(const hash_t *);

	extern void sha256Free(hash_t *);
	extern void sha256Delete(hash_t *);

	extern uint32_t sha256Mod(const hash_t *, uint32_t);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
