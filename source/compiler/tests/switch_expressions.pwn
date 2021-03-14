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
}
