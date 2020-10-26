enum Enum0 { e0, e1 };
enum Enum1
{
	e0,
	e0, // error 021: symbol already defined: "e0"
	e1
};

main(){}
