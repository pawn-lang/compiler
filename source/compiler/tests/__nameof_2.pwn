new symbol_1 = 4;

TestFunc(local_1)
{
	new dest[32];
	dest = __nameof(symbol_1);
	dest = __nameof(main);
	dest = __nameof(local_1);
	dest = __nameof(DoesntExist);
	dest = __nameof(local_3);
}

main()
{
	TestFunc(0);
}

