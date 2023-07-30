#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include <cjson/double_dabble.h>

typedef enum calc_flags
{
	SUB,
	SUM,
} CalcFlags;

bigint_t *newBigInt(uint64_t size, bigint_t *dest)
{
	bigint_t *new_bigint = dest;
	if (!new_bigint)
	{
		new_bigint = (bigint_t*)malloc(sizeof(bigint_t));
		if (!new_bigint)
			return NULL;
	}

	new_bigint->bin = size ? (char*)malloc(sizeof(char) * size) : NULL;

	if (!new_bigint->bin)
	{
		free(new_bigint);

		return NULL;
	}

	new_bigint->length = size;
	new_bigint->isNegative = 0;

	memset(new_bigint->bin, 0, sizeof(char) * size);

	return new_bigint;
}

void freeBigInt(bigint_t *bigint)
{
	free(bigint->bin);

	return;
}

void deleteBigInt(bigint_t *bigint)
{
	freeBigInt(bigint);
	free(bigint);

	return;
}

bigint_t *convertToBigInt(char *number, bigint_t *dest)
{
	if (!number)
		return NULL;

	uint64_t length = strlen(number);
	uint64_t size = (length+1)/2;
	bigint_t *bigint = newBigInt(size, dest);
	char *tmp_bin = (char*)malloc(sizeof(char) * size);

	memset(tmp_bin, 0, sizeof(char) * size);

	for (uint64_t i = 0; i < length; i++)
	{
		if (number[i] - '0' > 9)
		{
			free(tmp_bin);
			
			if (dest)
				freeBigInt(bigint);
			else 
				deleteBigInt(bigint);

			return NULL;
		}
	}

	for (uint64_t i = size; i-->0;)
	{
		uint8_t i1 = (i == 0 && length&1) ? 0 : number[length&1 ? i*2-1 : i*2] - '0';
		uint8_t i2 = number[length&1 ? i*2 : i*2+1] - '0';
		tmp_bin[i] = (i1 << 4) | i2;
	}

	for (uint64_t i = 0; i < size; i++)
	{
		for (uint64_t j = 0; j < 8; j++)
		{
			char tmp_bit1 = 0;
			char tmp_bit2 = 0;

			for (uint64_t g = 0; g < size; g++)
			{
				tmp_bit1 = tmp_bin[g]&1;
				tmp_bin[g] = ((tmp_bin[g] >> 1) & ~(0x80)) | (tmp_bit2 << 7);
				tmp_bit2 = tmp_bit1;
				for (uint64_t h = 0; h < 2; h++)
				{
					uint8_t num = GETGRP(tmp_bin[g], h);
					if (num >= 8)
					{
						CLRGRP(tmp_bin[g], h);
						SETGRP(tmp_bin[g], num-3, h);
					}
				}
			}
			
			for (uint64_t g = 0; g < size; g++)
			{
				tmp_bit1 = bigint->bin[g]&1;
				bigint->bin[g] = ((bigint->bin[g] >> 1) & ~(0x80)) | (tmp_bit2 << 7);
				tmp_bit2 = tmp_bit1;
			}
		}
	}

	free(tmp_bin);

	return bigint;
}

static bigint_t *calc(bigint_t *num1, bigint_t *num2, CalcFlags flag)
{
	if (!num1 || !num2)
		return NULL;
	
	char tmp = 0, fi1 = 1, fi2 = 1;
	uint64_t i1 = num1->length, i2 = num2->length;
	uint64_t size = ((i1 < i2) ? i2 : i1) + flag;
	uint64_t g = size;
	bigint_t *dest = newBigInt(size, NULL);

	if (!dest)
		return NULL;

	for (; g-->0;)
	{
		i1 -= fi1;
		i2 -= fi2;

		for (uint8_t j = 0; j < 8; j++)
		{
			char b1 = (fi1) ? GETBIT(num1->bin[i1], j) : 0, b2 = (fi2) ? GETBIT(num2->bin[i2], j) : 0;

			if (b1 ^ b2 ^ tmp)
				SETBIT(dest->bin[g], j);
			else
				CLRBIT(dest->bin[g], j);

			b1 = (flag) ? b1 : ~b1&1;
			tmp = (b1 & (b2 | tmp)) | (b2 & tmp);
		}
		
		fi1 = (i1 > 0) ? fi1 : 0;
		fi2 = (i2 > 0) ? fi2 : 0;
	}

	return dest;
}

bigint_t *addBigInt(bigint_t *num1, bigint_t *num2)
{
	return calc(num1, num2, ~(num1->isNegative ^ num2->isNegative) & 1);
}

bigint_t *subBigInt(bigint_t *num1, bigint_t *num2)
{
	return calc(num1, num2, (num1->isNegative ^ num2->isNegative) & 1);
}
