Tag:operator=(a) return Tag:a;
operator=(Tag:a) return _:a;

/* the compiler should warn about this operator being unused,
 * as normally it shouldn't be called on tag-matched returns */
Tag:operator=(Tag:a) return a;

Tag:Func1()
	return 7; /* `Tag:operator=(a)` */

Func2()
	return Tag:7; /* operator=(Tag:a) */

Tag:Func3()
	return Tag:7; /* neither operator should be called here (matching tags) */

Tag2:Func4()
	return Tag2:7; /* neither operator should be called here (no suitable operator) */

main()
{
	Func1();
	Func2();
	Func3();
	Func4();
}
