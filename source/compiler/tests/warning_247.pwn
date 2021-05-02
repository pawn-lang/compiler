main()
{
	new bool:a = true, bool:b = false;
	new u1 = 1, u2 = 0;

	if (u1) {}
	if (!u1) {}
	if (~u1) {}
	if (-u1) {}
	if (++u1) {}
	if (u1++) {}
	if (--u1) {}
	if (u1--) {}
	if (u1 || u2) {}
	if (u1 ^ u2) {}
	if (u1 && u2) {}
	if (u1 == u2) {}
	if (u1 != u2) {}
	if (u1 * u2) {}
	if (u1 / u2) {}
	if (u1 % u2) {}
	if (u1 + u2) {}
	if (u1 - u2) {}
	if (u1 << u2) {}
	if (u1 >> u2) {}
	if (u1 >>> u2) {}

	if (a) {}
	if (!a) {}
	if (a || b) {}
	if (a ^ b) {}
	if (a && b) {}
	if (a == b) {}
	if (a != b) {}
	if (a <= b) {} // warning 247: use of operator "<=" on "bool:" values
	if (a >= b) {} // warning 247: use of operator ">=" on "bool:" values
	if (a < b) {}  // warning 247: use of operator "<" on "bool:" values
	if (a > b) {}  // warning 247: use of operator ">" on "bool:" values
	if (~a) {}     // warning 247: use of operator "~" on a "bool:" value; did you mean to use operator "!"?
	if (-a) {}     // warning 247: use of operator "-" on a "bool:" value; did you mean to use operator "!"?
	if (++a) {}    // warning 247: use of operator "++" on a "bool:" value
	if (a++) {}    // warning 247: use of operator "++" on a "bool:" value
	if (--a) {}    // warning 247: use of operator "--" on a "bool:" value
	if (a--) {}    // warning 247: use of operator "--" on a "bool:" value
	if (a * b) {}  // warning 247: use of operator "*" on "bool:" values
	if (a / b) {}  // warning 247: use of operator "/" on "bool:" values
	if (a % b) {}  // warning 247: use of operator "%" on "bool:" values
	if (a + b) {}  // warning 247: use of operator "+" on "bool:" values
	if (a - b) {}  // warning 247: use of operator "-" on "bool:" values
	if (a << b) {} // warning 247: use of operator "<<" on "bool:" values
	if (a >> b) {} // warning 247: use of operator ">>" on "bool:" values
	if (a >>> b) {}// warning 247: use of operator ">>>" on "bool:" values

	if (~(u1 & u2)) {} // bitwise AND, the result is not forced to be "bool:"-tagged
	if (~(u1 && u2)) {} // warning 247: use of operator "~" on a "bool:" value; did you mean operator "!"?
}
