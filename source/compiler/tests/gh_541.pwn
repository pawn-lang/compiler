main()
{
	new const bool:TRUE = true;
	if (TRUE)
	{
		const CONSTVAL = 0;
		enum
		{
			ENUM_ITEM1
		};
		if (TRUE)
		{
			return CONSTVAL;
		}
		if (TRUE)
		{
			return ENUM_ITEM1;
		}
	}
	if (TRUE)
		return CONSTVAL; // error 017: undefined symbol "CONSTVAL"
	return ENUM_ITEM1; // error 017: undefined symbol "ENUM_ITEM1"
}
