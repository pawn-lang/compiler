#include <console>

// The branches of a ternary operator may use different amounts of heap
// space (e.g. an array-returning function call vs. a local array). The
// compiler must equilibrate the heap usage of both branches, regardless of
// which branch is taken, otherwise the heap is restored by a wrong amount
// after the expression.

stock A() { new a[8] = "aaaa"; return a; }
stock B() { new b[8] = "bb"; return b; }

stock Both(x)
{
	new r[8];
	new s[8] = "ss";
	r = x ? A() : s;
	printf("%s\n", r);
	r = x ? s : B();
	printf("%s\n", r);
	r = x ? A() : B();
	printf("%s\n", r);
	r = x ? "tt" : "ff";
	printf("%s\n", r);
	return r[0];
}

main()
{
	Both(1);
	Both(0);
}
