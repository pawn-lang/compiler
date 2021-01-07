#pragma warning disable 218 // old style prototypes used with optional semicolons

forward stock Func1();
stock Func1();
stock Func1(){}

forward public Func2();
public Func2();
public Func2(){}

forward Func3() __pragma("unused");
Func3();
Func3(){}

forward static Func4() __pragma("unused");
static Func4();
static Func4(){}

native NativeFunc();

// `const` is illegal in forward declarations.
forward const ConstFunc1();        // error 010: invalid function or declaration
forward static const ConstFunc2(); // error 010: invalid function or declaration
native const NativeConstFunc1();   // error 010: invalid function or declaration

// Combinations of `static`+`public` and `stock`+`public` are illegal.
forward static public StaticPublicFunc1(); // error 042: invalid combination of class specifiers
forward public static StaticPublicFunc2(); // error 042: invalid combination of class specifiers
static public StaticPublicFunc1();         // error 042: invalid combination of class specifiers
public static StaticPublicFunc2();         // error 042: invalid combination of class specifiers
forward stock public StockPublicFunc1();   // error 042: invalid combination of class specifiers
forward public stock StockPublicFunc2();   // error 042: invalid combination of class specifiers
stock public StockPublicFunc1();           // error 001: expected token: ";", but found "("
public stock StockPublicFunc2();           // error 001: expected token: ";", but found "("

// None of the other class specifiers can be used in combination with `native`.
native stock NativeFunc2();  // error 042: invalid combination of class specifiers
native public NativeFunc3(); // error 042: invalid combination of class specifiers
native static NativeFunc4(); // error 042: invalid combination of class specifiers
stock native NativeFunc5();  // error 010: invalid function or declaration
public native NativeFunc5(); // error 010: invalid function or declaration
static native NativeFunc5(); // error 010: invalid function or declaration

// Names starting with '@' imply `public`, which is correct when used
// in combination with an actual "public" specifier or without any specifiers
// at all, but shouldn't work with `static` and `native`.
forward @Func1();
@Func1();
@Func1(){}
forward public @Func2();
public @Func2();
public @Func2(){}
forward stock @Func3();  // error 042: invalid combination of class specifiers
stock @Func3();          // error 042: invalid combination of class specifiers
stock @Func3(){}         // error 042: invalid combination of class specifiers
forward static @Func4(); // error 042: invalid combination of class specifiers
static @Func4();         // error 042: invalid combination of class specifiers
static @Func4(){}        // error 042: invalid combination of class specifiers
native @NativeFunc();    // error 042: invalid combination of class specifiers

main(){}
