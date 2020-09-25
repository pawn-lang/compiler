enum (<<= 1)
{
	e1Elem1 = 0, // warning 245: enum increment "<<= 1" has no effect on zero value (symbol "e1Elem1")
	e1Elem2
};

enum (*= 1)
{
	e2Elem1 = 0, // warning 245: enum increment "*= 1" has no effect on zero value (symbol "e2Elem1")
	e2Elem2
};

enum (<<= 1)
{
	e3Elem1 = 1, // shouldn't warn on this line (enum element has a non-zero value)
	e3Elem2
};

enum (*= 1)
{
	e4Elem1 = 1, // shouldn't warn on this line (enum element has a non-zero value)
	e4Elem2
};

enum (<<= 0)
{
	e5Elem1 = 0, // shouldn't warn on this line (the increment is 0)
	e5Elem2
};

enum (*= 0)
{
	e6Elem1 = 0, // shouldn't warn on this line (the increment is 0)
	e6Elem2
};

enum (<<= 1)
{
	e7Elem1 = 0, // shouldn't warn on this line (no next element)
};

enum (*= 1)
{
	e8Elem1 = 0, // shouldn't warn on this line (no next element)
};

main(){}
