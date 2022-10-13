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

#ifdef SHA256_CLASS

typedef struct SHA256 sha256_class;

struct SHA256 {
	hash_t *(*getHash)(const wchar_t*);
	unsigned char *(*toString)(const hash_t*);
	int (*compare)(const hash_t*, const hash_t*);
	int (*print)(const hash_t*);
};

sha256_class SHA256 = { &str2sha256, &hash2str, &cpmhash, &print_hash };

#endif

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
