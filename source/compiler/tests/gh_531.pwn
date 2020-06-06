#pragma option -d0

stock operator~(Tag:values[], count)
{
    #pragma unused values, count
}

main()
{
	static const bool:TRUE = true;
	__emit nop;
	while (TRUE)
	{
		new Tag:var2 = Tag:2;
		if (TRUE)
		{
			new Tag:var3 = Tag:3;
			if (TRUE)
			{
				break;
			}
		}
	}

	__emit nop;
	new Tag:var1;
	while (TRUE)
		continue;

	__emit nop;
	for (new Tag:i = Tag:0; TRUE; )
		continue;

	__emit nop;
}
