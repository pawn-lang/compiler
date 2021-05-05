forward [2][3]Func();
Func()
{
	new a[2][3];
	return a;
}

native [2][3]NativeFunc();

main()
{
	new a[2][3];
	a = Func();
	a = NativeFunc();
	return a[0][0];
}
