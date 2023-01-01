GetDiscount(numitems)
{
	return switch (numitems;
		1, 2, 3: 0;
		4..10:   10;
		_:       15;
	);
}

GetDiscount2(numitems)
{
	// Alternative syntax with implicit default case and optional ";" after it.
	assert(numitems > 0);
	return switch (numitems;
		1, 2, 3: 0;
		4..10:   10;
		         15
	);
}

main()
{
	GetDiscount(1);
	GetDiscount2(1);
}
