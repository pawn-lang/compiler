enum
{
	CONST1,
	Float:CONST2, // error
	CONST3
};

enum Float:
{
	CONST4,
	_:CONST5, // error
	CONST6
};

enum eNamed1
{
	CONST7,
	Float:CONST8,
	CONST9
};

enum Float:eNamed2
{
	CONST10,
	Float:CONST11,
	CONST12
};

main(){}
