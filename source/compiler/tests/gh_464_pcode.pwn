Tag:operator=(a) return Tag:a;
operator=(Tag:a) return _:a;

__emit nop; /* mark the start of checked P-code */

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
