enum TEST
{
	ABC1,
	ABC2
};

__static_check(_:TEST == 2, "size of \"TEST\" is not 2");
__static_check(_:TEST == 123, "size of \"TEST\" is not 123"); // warning 249: check failed: size of "TEST" is not 123

new Test[(__static_check(1), TEST)];
#pragma unused Test

__static_check(sizeof(Test) == 2, "size of \"Test\" array is not 2");
__static_check(sizeof(Test) == 123, "size of \"Test\" array is not 123"); // warning 249: check failed: size of "Test" array is not 123

main()
{
	const cval = 1;
	new var = 1234;
	__static_check(  cval - 1  ); // warning 249: check failed: cval - 1023
	__static_check(0, "test_string"); // warning 249: check failed: test_string
	__static_check(0, "\\ test \\ \" abc \""); // warning 249: check failed: \ test \ " abc "
	__static_check(0, "string" " concatenation " "test"); // warning 249: check failed: string concatenation test
	__static_check(0, #stringization " test"); // warning 249: check failed: stringization test

	new arr[(__static_check(32%2 == 0, "array size must be even"), 32)];
	new arr2[(__static_check(33%2 == 0, "array size must be even"), 33)]; // warning 249: check failed: array size must be even
	#pragma unused arr
	#pragma unused arr2

	__static_check(1 && __static_check(cval == cval) || false);
	__static_check(1 && __static_check(cval != cval) && false); // warning 249: check failed: cval != cval
	                                                            // warning 249: check failed: 1 && __static_check(cval != cval) && false
	__static_check(__static_check(cval != cval, "inner check") || true, "outer check");  // warning 249: check failed: inner check
	__static_check(__static_check(cval == cval, "inner check") && false, "outer check"); // warning 249: check failed: outer check

	__static_check((0 + 1 + 2 + 3 + 4
	               + 5 + 6 + 7 + 8 + 9) * 0); // warning 249: check failed: (0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9) * 0

	__static_check(var); // error 008: must be a constant expression; assumed zero
	__static_check(cval, var); // error 001: expected token: "-string-", but found "-identifier-"
	__static_check(cval, !"test"); // error 001: expected token: "-string-", but found "!"
	__static_check(cval, !""); // error 001: expected token: "-string-", but found "!"

#if defined SecondPass
	// Only check assertions on the 2'nd pass, otherwise
	// warnings from the above checks won't be displayed.
	__static_assert(1, "this is not zero");
	__static_assert(0, "this is zero"); // fatal error 110: assertion failed: this is zero
#endif
}

stock SecondPass(){}
