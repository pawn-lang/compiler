// The result of a conditional expression carries only the array size of the
// first operand, so when the operands are arrays of different sizes, the
// result is copied (array assignment) or passed (array argument of a fixed
// size) with a wrong size. This must be an error, like it is for a plain
// array assignment or function call with a mismatching array size.

GetString1()
{
	new s[1] = {'0'};
	return s;
}

GetString2()
{
	new s[2] = "0";
	return s;
}

Unsized(const str[])
{
	#pragma unused str
}

Sized(const str[2])
{
	#pragma unused str
}

new g2a[2] = "1";
new g2b[2] = "2";
new g3[3] = "12";
new m[2][2];

main()
{
	new x = 0;
	new s[2];
	new t[4];

	s = x ? "1" : GetString1();     // error 047 (the case from GH #49)
	s = x ? "1" : GetString2();     // ok
	s = x ? "1" : "22";             // error 047 (literal strings of different sizes)
	t = x ? "1" : "22";             // error 047 (both fit, but the copy size is fixed)
	t = x ? "1" : "2";              // ok (strings smaller than the destination)
	s = x ? g2a : g3;               // error 047
	s = x ? g2a : g2b;              // ok
	s = x ? (x ? g2a : g3) : g2b;   // error 047 (nested, first operand)
	s = x ? g2b : (x ? g2a : g3);   // error 047 (nested, second operand)
	Unsized(x ? g2a : g3);          // ok (argument without a fixed size)
	Sized(x ? g2a : g3);            // error 047
	Sized(x ? g2a : g2b);           // ok
	s = x ? m : g2a;                // error 048 (different number of dimensions)
}
