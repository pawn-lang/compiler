#pragma option -d0

main()
{
	new x = 0;
	switch (x)
	{
		case 2:
		{
			x++;
			// NOTE: This case is not "fall-through", so the compiler
			// should generate a jump instruction here.
		}
		case 1:
		{
			switch (x)
			{
				case 1:
				{
					x++;
					__fallthrough;
				}
				case 0:
				{
					x++;
				}
			}
			__fallthrough;
		}
		case 0:
		{
			x++;
			// NOTE: This "__fallthrough" should be silently ignored,
			// as we'll still need to jump over the case table.
			__fallthrough;
		}
	}
}
