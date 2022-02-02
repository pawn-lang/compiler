#include <console>
#include "const_static.inc"

main()
{
	UseConstVal();
	printf("%d", const_val); // error 017: undefined symbol "const_val"

	// A combination of "const static" can't be used for local constants.
	const static local_const = 0; // error 020: invalid symbol name ""
	                              // warning 215: expression has no effect
	                              // warning 203: symbol is never used: ""
}
