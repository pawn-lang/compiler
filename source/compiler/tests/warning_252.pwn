main()
{
	new a = 1, b = 2, c = 3, d = 4, e = 5;
	a++;      // warning 252: variable has its value modified but never used: "a"
	b--;      // warning 252: variable has its value modified but never used: "b"
	++c;      // warning 252: variable has its value modified but never used: "c"
	--d;      // warning 252: variable has its value modified but never used: "d"

	// "e" is used as a return value, so it shouldn't trigger warning 252
	if (e--)
		return e;
	// but here it should cause a warning, as the post-increment is redundant
	return e++;
}
