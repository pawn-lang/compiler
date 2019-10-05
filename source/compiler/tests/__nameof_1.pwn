const SYMBOL_1 = 4;

new symbol_2 = 4;

#pragma unused symbol_2

enum SYMBOL_3
{
	SYMBOL_4,
};

UnusedFunc()
{
	return 5;
}

#pragma unused UnusedFunc

TestFunc(local_1, const local_2[])
{
	#pragma unused local_1, local_2
	new dest[32];
	dest = __nameof(SYMBOL_1);
	dest = __nameof(symbol_2);
	dest = __nameof(SYMBOL_3);
	dest = __nameof(SYMBOL_4);
	dest = __nameof(local_1);
	dest = __nameof(local_2);
	dest = __nameof(dest);
	dest = __nameof(TestFunc);
	dest = __nameof(UnusedFunc);
}

main()
{
	TestFunc(0, "__nameof");
}

