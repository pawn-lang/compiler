GetDiscount(numitems)
{
	return switch (numitems;
		1, 2, 3: 0;
		4..10:   10;
		_:       15;
	);
}

main()
{
	GetDiscount(1);
}
