#pragma warning disable 218 // "old style prototypes used with optional semicolumns"
forward Func(const a[] = { 1, 2, 3 }); // original declaration

forward Func(const a[] = { 1, 2, 3 });
forward Func(const a[] = { 2, 3, 4 }); // error 025

stock Func(const a[] = { 1, 2, 3 });
stock Func(const a[] = { 2, 3, 4 }); // error 025

stock Func(const a[] = { 3, 4, 5 }) // error 025
{
	return a[0];
}

main(){}
