main()
{
	new x = 0;
	switch (x)
	{
		case 0:
		{
			x++;
			__fallthrough;
		}
		case 1:
		{
			__fallthrough; // error
			x++;
		}
		default:
		{
			x++;
			__fallthrough; // error
		}
	}
	__fallthrough; // error
}
