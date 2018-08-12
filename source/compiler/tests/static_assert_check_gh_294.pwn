enum TEST {
	ABC1,
	ABC2
}

__static_check(_:TEST == 2, "size of \"TEST\" is not 2");
__static_check(_:TEST == 123, "size of \"TEST\" is not 123");

new Test[(__static_check(1), TEST)];
#pragma unused Test

__static_check(sizeof(Test) == 2, "size of \"Test\" array is not 2");
__static_check(sizeof(Test) == 123, "size of \"Test\" array is not 123");

main() {
	const var = 1023;
	new var2 = 1234;
	__static_check(0);
	__static_check(0, "test_string");
	__static_check(0, "\\ test \\ \" abc \"");
	__static_check(var%512 - 2*256 + 1, "this is zero");
	__static_check(var - 1024, "this is not zero");
	__static_check(var ^ var + 10 - 2*4 - 2);

	new arr[(__static_check(32%2 == 0, "array size must be even"), 32)];
	new arr2[(__static_check(33%2 == 0, "array size must be even"), 33)];
	#pragma unused arr
	#pragma unused arr2

	//__static_check 10 == 55 && 0, 55;

	__static_check(var2);
	__static_check(var, asd");
	__static_check(var, "asd);
}
