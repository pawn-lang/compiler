#include <console>

main()
{
	// Case 1: Both operands are compile-time constants
	printf("%d", 1 / 0); // error 094
	printf("%d", 1 % 0); // error 094
	printf("%d", 1 / 1);
	printf("%d", 1 % 1);

	// Case 2: Only the divisor is a constant
	new var = 0;
	printf("%d", var / 0); // error 094
	printf("%d", var % 0); // error 094
	printf("%d", var / 1);
	printf("%d", var % 1);

	printf("%d", 1 / var); // Just to make sure the error works only
	printf("%d", 1 % var); // if the divisor is a constant value
}
