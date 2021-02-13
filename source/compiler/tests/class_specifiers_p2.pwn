#pragma warning disable 218 // old style prototypes used with optional semicolons

// This should cause an error, as public variables can't be stock
stock @var1 = 0;         // error 042: invalid combination of class specifiers

// This shouldn't cause an assertion failure
static @var2 = 0;        // error 042: invalid combination of class specifiers

// The compiler should expect the function tag and the return array size AFTER
// the class specifiers and `__pragma`
forward Tag:[2] static stock Func();  // error 010: invalid function or declaration
forward static stock Tag:[2] Func2(); // OK

main(){}
