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
	new __pragma("unused") retval = (NakedFunc(), NakedFunc2());

	// Make sure the compiler doesn't crash on an empty `__pragma`.
	__pragma(""); // warning 207: unknown #pragma
	__pragma(" "); // warning 207: unknown #pragma

	// `#pragma` warns about extra characters after the option name,
	// so `__pragma` should do the same.
	__pragma("unused b"); // warning 207: unknown #pragma
	// But it shouldn't warn if there are only trailing whitespaces after the name.
	__pragma("naked   ");

	// Warning 200 is going to be temporarily disabled, so the compiler
	// shouldn't warn about the first variable having too long name.
	// Also the parameter strings contain excess whitespaces, the compiler
	// should ignore them.
	__pragma("warning push ", "warning disable 200  ");
	new
		long_name_zzzzzzzzz_zzzzzzzzz_zzzzzzzzz_ __pragma("unused"),
		// `__pragma("warning")` takes effect immediately, so the compiler
		// should warn that the name of the next variable is too long.
		// warning 200: symbol "long_name2_zzzzzzzz_zzzzzzzzz_z" is truncated to 31 characters
		__pragma("warning enable 200   ", "unused") long_name2_zzzzzzzz_zzzzzzzzz_zzzzzzzzz_ __pragma("unused");
	__pragma("warning pop    ");

	// Warn if the parameter of "warning disable" is not a number.
	__pragma("warning enable    "); // warning 207: unknown #pragma
	__pragma("warning enable  a "); // warning 207: unknown #pragma
	__pragma("warning enable  - "); // warning 207: unknown #pragma
}
