#ifndef SHA256_H_
#define SHA256_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t *str2sha256(const wchar_t *key);
extern unsigned char *hash2str(const uint32_t *hash);
extern int cmphash(const uint32_t *hash1, const uint32_t *hash2);
extern int print_hash(const uint32_t *hash);

#ifdef SHA256_CLASS

typedef struct SHA256 sha256_class;

struct SHA256 {
	uint32_t *(*getHash)(const wchar_t*);
	unsigned char *(*toString)(const uint32_t*);
	int (*compare)(const uint32_t*, const uint32_t*);
	int (*print)(const uint32_t*);
};

sha256_class SHA256 = { &str2sha256, &hash2str, &cpmhash, &print_hash };

#endif

#ifdef __cplusplus
}
#endif

#endif // SHA256_H_
