#ifndef SHA256_H_
#define SHA256_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hash_t;

extern hash_t *str2sha256(const char*);
extern hash_t *wstr2sha256(const wchar_t*);
extern wchar_t *hash2str(const hash_t*);
extern int compareHash(const hash_t*, const hash_t*);
extern int printHash(const hash_t*);
extern void freeHash(hash_t*);
extern uint32_t sha256Mod(const hash_t*, uint32_t k);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
