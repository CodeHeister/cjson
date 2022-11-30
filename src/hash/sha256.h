#ifndef SHA256_H_
#define SHA256_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hash_t;

extern hash_t *str2sha256(const wchar_t *key);
extern wchar_t *hash2str(const hash_t *hash);
extern int cmphash(const hash_t *hash1, const hash_t *hash2);
extern int print_hash(const hash_t *hash);
extern uint32_t hash_mod(const hash_t *hash, uint32_t k);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
