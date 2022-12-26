test_basic()
{
	new a = 1, b = 2, c = 3, d = 4;
	a++;      // warning 252: variable has its value modified but never used: "a"
	b--;      // warning 252: variable has its value modified but never used: "b"
	++c;      // warning 252: variable has its value modified but never used: "c"
	--d;      // warning 252: variable has its value modified but never used: "d"
}

test_retval()
{
	new x = 1;
	// "x" is used after it's modified, so it shouldn't trigger warning 252
	if (x--)
		return x;
	// but here it should cause a warning, as the post-increment is redundant
	return x++; // warning 252: variable has its value modified but never used: "x"
}

test_pragma_unused_unread()
{
	new x = 1, y = 2;
	x += 2;
	y -= 1;
	#pragma unused x
	#pragma unread y
}

main()
{
	test_basic();
	test_retval();
	test_pragma_unused_unread();
}
