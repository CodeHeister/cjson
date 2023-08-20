#include <stdio.h>
#include <stddef.h>

#include <cjson/double_dabble.h>
#include <cjson/debug.h>

int main()
{
	bigint_t *num1 = convertToBigInt("999");
	bigint_t *num2 = convertToBigInt("-99");
	bigint_t *num3 = addBigInt(num1, num2);
	bigint_t *num4 = subBigInt(num1, num2);

	printBytesAsBin(num1->bin, num1->length);
	printBytesAsBin(num2->bin, num2->length);
	printBytesAsBin(num3->bin, num3->length);
	printBytesAsBin(num4->bin, num4->length);
	
	deleteBigInt(num1);
	deleteBigInt(num2);
	deleteBigInt(num3);
	deleteBigInt(num4);

	return 0;
}
