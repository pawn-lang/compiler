Func1(const const a) return a; // error 042: invalid combination of class specifiers
Func2(const a) return a;
Func3(a) return a;

main()
{
	Func1(0);
	Func2(0);
	Func3(0);
}

