enum
{
	CONST1_1,
	CONST1_2,
	CONST1_3,
	CONST1_4
};

enum eNamedEnum
{
	CONST2_1,
	CONST2_2,
	CONST2_3,
	CONST2_4 = CONST2_3,
	CONST2_5
};

main()
{
	new var1 = CONST1_1;
	new eNamedEnum:var2 = CONST2_1;

	switch (var1)
	{
		// Warning 241 should NOT be printed in this case, as the constants
		// belong to an unnamed (anonymous) enumeration.
		case CONST1_1, CONST1_2: {}
	}

	switch (var2)
	{
		// Two or less elements of a named enum are not covered by a switch
		// statement; warning 241 must be printed in this case.
		// Also, 'CONST2_4' has the same value as 'CONST2_3', so the said
		// warning must be printed only for 'CONST2_3' and 'CONST2_5'.
		case CONST2_1, CONST2_2: {}
	}

	switch (var2)
	{
		// There is a default case, which means all possible values are covered,
		// so warning 241 should NOT be printed in this case.
		case CONST2_1, CONST2_2: {}
		default: {}
	}

	switch (var2)
	{
		// More than two enum elements are missing, which might be intentional,
		// so warning 241 should NOT be printed in this case.
		case CONST2_1: {}
	}
}
