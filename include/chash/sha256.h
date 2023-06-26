#ifndef SHA256_H_
#define SHA256_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hash_t;

extern hash_t *str2sha256(const char *key);
extern hash_t *wstr2sha256(const wchar_t *key);
extern wchar_t *hash2str(const hash_t *hash);
extern int compareHash(const hash_t *hash1, const hash_t *hash2);
extern int printHash(const hash_t *hash);
extern uint32_t sha256Mod(const hash_t *hash, uint32_t k);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
