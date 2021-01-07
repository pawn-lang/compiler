#pragma warning disable 218 // old style prototypes used with optional semicolons

// This should cause an error, as public variables can't be stock
stock @var1 = 0;         // error 042: invalid combination of class specifiers

// This shouldn't cause an assertion failure
static @var2 = 0;        // error 042: invalid combination of class specifiers

main(){}
