// Case 1: The compiler should warn us if we are redefining an empty named enum
enum test1 {};
enum test1 {}; // warning 201: redefinition of constant/macro (symbol "test1")

// Case 2: The compiler shouldn't crash because of "test2" being redefined
enum _:test2 {};
enum
{
    test2 // warning 201: redefinition of constant/macro (symbol "test2")
};

// Case 3: The compiler shouldn't crash because of "test3" being redefined
// (this is different from case 2 since we're redefining the root symbol
// of the current enum, not of another one)
enum _:test3
{
	test3 // warning 201: redefinition of constant/macro (symbol "test3")
};

main(){}
