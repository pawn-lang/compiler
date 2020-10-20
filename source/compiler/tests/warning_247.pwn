main()
{
	new a = 0, bool:b = false, c;
	if (a) {}
	if (b) {}
	if (!a) {}
	if (!b) {}
	if (~a) {} // bitwise inversion on an untagged value is OK
	if (~b) {} // warning 247: use of operator "~" on a "bool:" value always results in "true"; did you mean operator "!"?
	if (~(a & c)) {} // bitwise AND, the result is not forced to be "bool:"-tagged
	if (~(a && c)) {} // warning 247: use of operator "~" on a "bool:" value always results in "true"; did you mean operator "!"?
}
