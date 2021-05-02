#pragma warning disable 218 // old style prototypes used with optional semicolons

// This should cause an error, as public variables can't be stock
stock @var1 = 0;         // error 042: invalid combination of class specifiers

// This shouldn't cause an assertion failure
static @var2 = 0;        // error 042: invalid combination of class specifiers

// The compiler should expect the function tag and the return array size AFTER
// the class specifiers and `__pragma`
forward Tag:[2] static stock Func();  // error 010: invalid function or declaration
forward static stock Tag:[2] Func2(); // OK

forward Func3();
public Func3();  // class specifier "public" is introduced; it will be required in the definition
Func3(){}        // error 025: function heading differs from prototype

static Func4();  // class specifier "static" is introduced
forward Func4(); // OK (class specifiers are only mandatory in function definitions, not declarations)
static Func4(){} // OK (class specifier "static" is in place)

static Func5(){} // Func5() is "finalized"; subsequent forward declarations
                 // for this function can't introduce any new class specifiers
forward static Func5(); // OK (no new class specifiers)
forward static stock Func5(); // error 025: function heading differs from prototype

#pragma unused Func4, Func5

main(){}
