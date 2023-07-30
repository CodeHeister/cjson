#include <stdio.h>
#include <stddef.h>

#include <cjson/double_dabble.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

int main()
{
	bigint_t *num1 = convertToBigInt("999", NULL);
	bigint_t *num2 = convertToBigInt("99", NULL);
	// num2->isNegative = 1;
	bigint_t *num3 = addBigInt(num1, num2);
	bigint_t *num4 = subBigInt(num1, num2);

	for (uint64_t g = 0; g < num1->length; g++)
	{
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(num1->bin[g]));
	}
	printf("\n");
	for (uint64_t g = 0; g < num2->length; g++)
	{
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(num2->bin[g]));
	}
	printf("\n");
	for (uint64_t g = 0; g < num3->length; g++)
	{
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(num3->bin[g]));
	}
	printf("\n");
	for (uint64_t g = 0; g < num4->length; g++)
	{
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(num4->bin[g]));
	}
	printf("\n");
	
	deleteBigInt(num1);
	deleteBigInt(num2);
	deleteBigInt(num3);
	deleteBigInt(num4);

	return 0;
}
