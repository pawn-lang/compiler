main()
{
	new x = 0;
	switch (x)
	{
		case 0:
		{
			x++;
			#pragma fallthrough
		}
		case 1:
		{
			#pragma fallthrough
			x++;
		}
		default:
		{
			x++;
			#pragma fallthrough // error
		}
	}
	#pragma fallthrough // error
}
