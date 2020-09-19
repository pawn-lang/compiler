native __pragma("deprecated - use OtherFunc() instead") Func(arg __pragma("deprecated"));
stock Func2(const __pragma("unread") arg = 0 __pragma("unwritten")) {}

// error 001: expected token: "-identifier-", but found "const"
// "__pragma" can't be used before the "const" specifier
stock Func3(__pragma("unread") const arg) {}

// error 001: expected token: "-identifier-", but found "__pragma"
// "__pragma" can't be used between the tag and the symbol name
stock Func4(Tag: __pragma("unread") arg) {}

operator~(Tag:val[],count) {}

NakedFunc()
{
	__pragma("naked", "deprecated - use NakedFunc2() instead");
}

__pragma("naked") NakedFunc2(__pragma("naked", "unused") arg = 0) {}

main()
{
	new __pragma("unwritten", "unread") a = 0;
	new b = 0 __pragma("unwritten", "unread");
	new __pragma("unwritten") c = 0 __pragma("unread");
	new d = 0 __pragma("unused");

	// warning 204: symbol is assigned a value that is never used: "e"
	new e __pragma("unwritten");

	// warning 203: symbol is never used: "f"
	// warning 203: symbol is never used: "operator~(Tag:)"
	new Tag:f __pragma("nodestruct");

	// warning 234: function is deprecated (symbol "Func") - use OtherFunc() instead
	Func(0);

	// NakedFunc() and NakedFunc2() are marked as "naked", so there should be
	// no warnings about them having to return a value.
	return NakedFunc(), NakedFunc2();
}
