#pragma option -d1 // for #assert
#pragma option -;+

enum myenum
{
	CONST1 = 1,
	CONST2, // 2
	CONST3  // 3
};

enum continue myenum (+= 2)
{
	CONST4, // 4
	CONST6  // 6
};
#assert CONST4 == 4
#assert CONST6 == 6

continue enum myenum (*= 3)
{
	CONST8, // 8
	CONST24 // 24
};
#assert CONST8 == 8
#assert CONST24 == 24


enum // anonymous enumeration
{
	ACONST1 = 1,
	ACONST2, // 2
	ACONST3  // 3
};

enum continue // error (no enum name)
{
	ACONST4
};

continue enum // error (no enum name)
{
	ACONST5
};

continue; // error ('continue' used out of context)

continue enum dialogid // the 'continue' keyword is pretty much ignored here
{
	DIALOG_NONE
};

public Func1();
public Func1()
{
	continue enum dialogid // error (enum continuation doesn't work inside functions)
	{
		DIALOG_ID1
	};
	#pragma unused DIALOG_ID1
}

main(){}
