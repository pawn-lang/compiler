Func(){ return 0; }

main()
{
	new x = 1, y = 2;

	// Case 1: The user could've meant to use '+' or some other operator
	// instead of ',', so the compiler should print a warning.
	if (x,y == 3) {} // warning 248: possible misuse of comma operator

	// Case 2: Functions may have side effects, so the use of ','
	// might be intended; we shouldn't warn in such cases.
	if (Func(),x) {}

	// Case 3: Sub-expression "x = 0" has a side effect; shouldn't warn.
	if ((x = 0),y) {}

	// Case 4: Sub-expression "y = 0" has a side effect, but it's located
	// after the ',', so the use of comma operator might be unintended.
	if (x,(y=0)) {} // warning 248: possible misuse of comma operator

	// Case 5: The expression is inside an extra pair of parentheses, by which
	// the user tells they know what they're doing; shouldn't warn in this case.
	if ((x,y)) {}

	#pragma unused x, y // avoid "unused assignment" warnings
}
