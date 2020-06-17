#include <console>

main()
{
	new var = 1;

	// Case 1: Both values are compile-time constants
	printf("%d", 1 << -1); // warning 241
	printf("%d", 1 << -2); // warning 241
	printf("%d", 1 << 0);
	printf("%d", 1 << 1);
	printf("%d", 1 << 30);
	printf("%d", 1 << 31);
	printf("%d", 1 << 32); // warning 241
	printf("%d", 1 << 33); // warning 241

	// Case 2: Only the shift count is constant
	printf("%d", var << -1); // warning 241
	printf("%d", var << -2); // warning 241
	printf("%d", var << 0);
	printf("%d", var << 1);
	printf("%d", var << 30);
	printf("%d", var << 31);
	printf("%d", var << 32); // warning 241
	printf("%d", var << 33); // warning 241

	printf("%d", 1 << var); // Just to make sure the warning works only
	                        // if the shift count is a constant value
}
