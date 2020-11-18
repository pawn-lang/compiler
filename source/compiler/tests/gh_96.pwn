#pragma option -O0 // disable optimizations

Func() return 1;

main()
{
	static x = 0, y;

	__emit nop;
	switch (x)
	{
		// The code for "Func()" call shouldn't be erased.
		case 0:  y = (Func(), 0);

		// The code for the increment shouldn't be erased.
		case 1:  y = (x++, 0);

		// The "1 + 2" part is constant, so its code can be safely discarded.
		default: y = (1 + 2, 0);
	}
	__emit nop;

	return y;
}
