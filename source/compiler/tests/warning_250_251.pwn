#include <core>
#include <file>

new glbvar = 0;

stock UseVarByRef(&arg)
	return arg;

#pragma warning disable 238 // "meaningless combination of class specifiers (const reference)"
stock UseVarByConstRef(const &arg)
	return arg;

main()
{
	new n = 0, m = 10;
	static st = 0;

	// Case 1: Variable is used inside a loop condition without being modified.
	while (n < 10) {}                      // warning 250: variable "n" used in loop condition not modified in loop body
	do {} while (n < 10);                  // warning 250: variable "n" used in loop condition not modified in loop body
	for (new i = 0, j = 0; i < 10; ++j) {} // warning 250: variable "i" used in loop condition not modified in loop body

	// Case 2: Variable is used inside a loop condition and modified in the loop body.
	while (n != 0) { n++; }
	do { n++; } while (n < 10);
	for (new i = 0; i < 10; ) { i++; }

	// Case 3: Variable is used inside a loop condition and modified in the
	// loop counter increment/decrement section.
	for (new i = 0; i < 10; i++) {}

	// Case 4: Variable is used and modified inside a loop condition.
	while (n++ != 0) {}
	while (++n != 0) {}
	do {} while (n++ != 0);
	do {} while (++n != 0);
	for (new i = 0; i++ < 10; ) {}
	for (new i = 0; ++i < 10; ) {}

	// Case 5: Same variable is used inside a loop condition more than once
	// and it's not modified.
	while (n == 0 || n < 10) {}            // warning 250: variable "n" used in loop condition not modified in loop body
	do {} while (n == 0 || n < 10);        // warning 250: variable "n" used in loop condition not modified in loop body
	for (new i = 0; i == 0 || i < 10; ) {} // warning 250: variable "i" used in loop condition not modified in loop body

	// Case 6: Same variable is used inside a loop condition more than once,
	// but it's modified.
	while (n == 0 || n < 10) { n++; }
	do { n++; } while (n == 0 || n < 10);
	for (new i = 0; i == 0 || i < 10; i++) {}

	// Case 7: Two variables are used inside a loop condition, both aren't modified.
	// Printing warning 250 for each unmodified variable wouldn't be productive, because:
	//   1. the user would be spammed with multiple warnings, which can be annoying;
	//   2. depending on the context, only one of the variables might be supposed
	//      to be modified, but the compiler can't know which one exactly.
	// Solution: introduce a warning that simply says that none of the variables
	// were modified, and let the user decide which variable should be modified.
	while (n < m) {}            // warning 251: none of the variables used in loop condition are modified in loop body
	do {} while (n < m);        // warning 251: none of the variables used in loop condition are modified in loop body
	for (new i = 0; i < m; ) {} // warning 251: none of the variables used in loop condition are modified in loop body

	// Case 7: Two variables are used in a loop condition, but one of them
	// is modified inside the loop body (or the loop counter increment/decrement
	// section of a "for" loop), and the other one is not modified.
	while (n < m) { ++n; }
	do { --m; } while (n < m);
	for (new i = 0; i < m; ) { i++; }
	for (new i = 0; i < m; i++) {}

	// Case 8: Two variables are used in a loop condition, but one of them
	// is being modified prior to being used, and the other one is not modified.
	while (++n < m) {}
	do {} while (++n < m);
	for (new i = 0; ++i < m; ) {}

	// Case 9: Two variables are used in a loop condition, but one of them
	// is static and it's modified inside the loop body, and the other one
	// is a stack variable and it's left unmodified.
	while (st < m) { st++; }

	// Case 10: Warnings 250 and 251 may be inaccurate when an array is indexed
	// inside a loop condition. The problem is that we can't memoize the array
	// that is being indexed by a variable, to unset the "uLOOPVAR" flag for the
	// array symbol later when the variable is modified.
	// This is why I had to completely disable those diagnostics for arrays.
	{
		new a[3] = { 0, 0, 1 };
		n = random(sizeof(a));
		while (a[n] == 0) // Shouldn't warn about "n" not being modified
			a[n] = random(3);
		for (new i = 0; a[i++] == 0; ) {} // shouldn't warn about "a" not being modified
		for (a[0] = 0, m = 10; a[0] < m; ++a[0]) {} // shouldn't warn about "m" not being modified
	}

	// Case 11: Just as with arrays, warnings 250 and 251 are disabled when
	// there's a function call inside a loop condition, as those diagnostics
	// may be inaccurate otherwise.
	new File:f = fopen("test.txt", io_read);
	new line[128];
	while (fread(f,line,sizeof(line),false) < m) {}     // shouldn't warn about "f" or "m" not being modified
	do {} while (fread(f,line,sizeof(line),false) < m); // shouldn't warn about "f" or "m" not being modified
	fclose(f);

	// Case 12: Warnings 250 and 251 shouldn't trigger when at least one global
	// variable is used inside the loop condition, as globals can be modified
	// from a function called from the loop body and currently there's no easy
	// way to track this.
	while (n < glbvar) {}
	do {} while (n < glbvar);
	for (new i = 0; i < glbvar; ) {}

	// Case 13: Warnings 250 and 251 shouldn't trigger when the loop counter
	// variable is passed to a function by reference.
	while (n < 10) UseVarByRef(n);
	while (n < m) UseVarByRef(n);

	// Case 14: While const references for single function arguments are
	// meaningless and there's warning 238 for this, such references still
	// shouldn't affect warnings 250 and 251, as variables passed by const
	// references aren't counted as modified.
	while (n < 10) UseVarByConstRef(n); // warning 250: variable "n" used in loop condition not modified in loop body
	while (n < m) UseVarByConstRef(n);  // warning 251: none of the variables used in loop condition are modified in loop body
}
