#ifndef BIGINT_H
#define BIGINT_H

#include <stddef.h>

typedef struct big_int_t {
    unsigned char* digits; // Array of digits 0-9.  Greater indices hold more significant digits.
	size_t num_digits; // Number of digits actually in the number.
    size_t length; // digits array has space for this many digits
    int is_negative; // Nonzero if this BigInt is negative, zero otherwise.
} big_int_t;



#endif
