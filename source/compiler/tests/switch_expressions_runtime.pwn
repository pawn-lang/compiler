#include <console>

GetDiscount(numitems)
{
	assert(numitems > 0);
	return switch (numitems;
		1, 2, 3: 0;
		4..10:   10;
		_:       15;
	);
}

PrintDiscount(numitems)
{
	printf("The discount for %d items is %d%c\n", numitems, GetDiscount(numitems), '%');
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

PrintDiscount2(numitems)
{
	printf("The discount for %d items is %d%c\n", numitems, GetDiscount2(numitems), '%');
}

main()
{
	PrintDiscount(1);
	PrintDiscount(3);
	PrintDiscount(4);
	PrintDiscount(7);
	PrintDiscount(10);
	PrintDiscount(11);
	PrintDiscount(20);
	PrintDiscount(50);
	PrintDiscount2(1);
	PrintDiscount2(3);
	PrintDiscount2(4);
	PrintDiscount2(7);
	PrintDiscount2(10);
	PrintDiscount2(11);
	PrintDiscount2(20);
	PrintDiscount2(50);
}
