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
stock Func3(); // class specifier "stock" is added
Func3(){} // OK (the function is implicitly defined as "stock")

forward Func4();
public Func4(); // class specifier "public" is added
Func4(){} // OK (the function is implicitly defined as "public")

forward Func5();
static Func5(); // class specifier "static" is added
Func5(){} // OK (the function is implicitly defined as "static")

// Func6() is declared as "static", so any subsequent re-declarations
// of this function with specifier "public" should be treated as errors
forward static Func6();
forward public Func6(); // error 025: function heading differs from prototype
public Func6(){}        // error 025: function heading differs from prototype

stock Func7(){} // Func4() is "finalized"; subsequent forward declarations
                // for this function can't introduce specifiers "static" and "public"
forward stock Func7(); // OK (no new class specifiers)
forward static stock Func7(); // error 025: function heading differs from prototype
static stock Func7(); // error 025: function heading differs from prototype

// Func8() is "finalized", but specifier "stock" can be added even after the definition,
// as it doesn't affect code generation (it only disables warning 203 for the function)
Func8(){}
forward stock Func8(); // OK

#pragma unused Func3, Func5, Func7, Func8

main(){}
