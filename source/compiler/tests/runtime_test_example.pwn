#include <console>

main() {
	new a[1] = {1};
	new i = 1;

	// When compiled with at least debug level 1 (default) the line below should produce:
	// Run time error 4: "Array index out of bounds"
	printf("%d", a[i]);
}
