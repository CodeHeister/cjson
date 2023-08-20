#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include <cjson/double_dabble.h>
#include <cjson/debug.h>

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

	new_bigint->size = size;
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

bigint_t *convert2toBigInt(char *number)
{
	uint64_t length = strlen(number);
	uint64_t size = (uint64_t)(length*0.125)+1;
	bigint_t *bigint = newBigInt(size, NULL);
	char *tmp_bin = (char*)malloc(sizeof(char) * size);

	memset(tmp_bin, 0, sizeof(char) * size);

	for (uint64_t i = 0; i < length; i++)
	{
		if (number[i] - '0' > 1)
		{
			free(tmp_bin);
			deleteBigInt(bigint);

			return NULL;
		}
	}

	return bigint;
}

bigint_t *convert8toBigInt(char *number)
{
	uint64_t length = strlen(number);
	uint64_t size = (uint64_t)(length*0.375)+1;
	bigint_t *bigint = newBigInt(size, NULL);
	char *tmp_bin = (char*)malloc(sizeof(char) * size);

	memset(tmp_bin, 0, sizeof(char) * size);

	for (uint64_t i = 0; i < length; i++)
	{
		if (number[i] - '0' > 7)
		{
			free(tmp_bin);
			deleteBigInt(bigint);

			return NULL;
		}
	}

	return bigint;
}

bigint_t *convert16toBigInt(char *number)
{
	uint64_t length = strlen(number);
	uint64_t size = (uint64_t)(length*0.5)+1;
	bigint_t *bigint = newBigInt(size, NULL);
	char *tmp_bin = (char*)malloc(sizeof(char) * size);

	memset(tmp_bin, 0, sizeof(char) * size);

	for (uint64_t i = 0; i < length; i++)
	{
		if (number[i] - '0' > 9 && number[i] - 'A' > 5 && number[i] - 'a' > 5)
		{
			free(tmp_bin);
			deleteBigInt(bigint);

			return NULL;
		}
	}

	return bigint;
}


bigint_t *convert10toBigInt(char *number)
{
	uint64_t length = strlen(number);
	uint64_t size1 = (uint64_t)(length*0.4152)+1;
	uint64_t size2 = (uint64_t)(length/2)+(length&1);
	bigint_t *bigint = newBigInt(size1, NULL);
	char *tmp_bin = (char*)malloc(sizeof(char) * size2);

	memset(tmp_bin, 0, sizeof(char) * size2);

	for (uint64_t i = 0; i < length; i++)
	{
		if (number[i] - '0' > 9)
		{
			free(tmp_bin);
			deleteBigInt(bigint);

			return NULL;
		}
	}

	for (uint64_t i = size2; i-->0;)
	{
		uint8_t i1 = (i == 0 && length&1) ? 0 : number[length&1 ? i*2-1 : i*2] - '0';
		uint8_t i2 = number[length&1 ? i*2 : i*2+1] - '0';
		tmp_bin[i] = (i1 << 4) | i2;
	}

	for (uint64_t i = 0; i < size1; i++)
	{
		for (uint64_t j = 0; j < 8; j++)
		{
			char tmp_bit1 = 0;
			char tmp_bit2 = 0;

			for (uint64_t g = 0; g < size2; g++)
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
			
			for (uint64_t g = 0; g < size1; g++)
			{
				tmp_bit1 = bigint->bin[g]&1;
				bigint->bin[g] = ((bigint->bin[g] >> 1) & ~(0x80)) | (tmp_bit2 << 7);
				tmp_bit2 = tmp_bit1;
			}

		}
	}

	free(tmp_bin);

	for (uint64_t i = 0; i < bigint->length; i++)
	{
		if (bigint->bin[i])
		{
			bigint->size = bigint->length-i;
			break;
		}
	}

	return bigint;
}

bigint_t *convertToBigInt(char *number)
{
	if (!number)
		return NULL;

	bigint_t *bigint = NULL;

	bool isNegative = 0;
	char *anchor = number;
	if (number[0] == '-')
	{
		anchor += 1;
		isNegative = 1;
	}

	if (anchor[0] == '0')
	{
		switch (anchor[1])
		{
			case 'b':
				bigint = convert2toBigInt(anchor+2);
				break;
			case 'o':
				bigint = convert8toBigInt(anchor+2);
				break;
			case 'x':
				bigint = convert16toBigInt(anchor+2);
				break;
			default:
				bigint = convert10toBigInt(anchor+1);
		}
	}
	else
	{
		bigint = convert10toBigInt(anchor);
	}

	bigint->isNegative = isNegative;

	return bigint;
}

static bigint_t *calc(bigint_t *num1, bigint_t *num2, bigint_t *num3,  CalcFlags flag)
{
	if (!num1 || !num2)
		return NULL;
	
	char tmp = 0, fi1 = 1, fi2 = 1;
	uint64_t i1 = num1->length, i2 = num2->length;
	uint64_t size = ((num1->size < num2->size) ? num2->size : num1->size) + flag;
	uint64_t g = size;
	bigint_t *dest = newBigInt(size, num3);

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
	return calc(num1, num2, NULL, ~(num1->isNegative ^ num2->isNegative) & 1);
}

bigint_t *subBigInt(bigint_t *num1, bigint_t *num2)
{
	return calc(num1, num2, NULL, (num1->isNegative ^ num2->isNegative) & 1);
}
