#include <console>
#include <file>
#include <string>
#include "gh_353_symbol_suggestions.inc"

forward test_nosuggest1();
public test_nosuggest1()
{
	// The compiler shouldn't suggest any name for this error
	// since "abcxyz" and "abcd" differ by more than 2 symbols.
	const abcd = 1;
	printf("%d\n", abcxyz);
	#pragma unused abcd
}

forward test_nosuggest2();
public test_nosuggest2()
{
	// There are no "()" after "length", so the compiler shouldn't suggest "flength".
	printf("%d\n", length);
}

forward test_nosuggest3();
public test_nosuggest3()
{
	// float.inc is not #included, so float() is not defined.
	// After the 1'st pass the compiler thinks float() is an unimplemented function,
	// so it shouldn't suggest variable "flt" in this case.
	new Float:flt;
	return float(0);
	#pragma unused flt
}

forward test_nosuggest4();
public test_nosuggest4()
{
	// "abc" is a label so the compiler shouldn't suggest its name
	// where a variable or named constant is expected.
abc:
	printf("%d\n", ab);
	printf("%d\n", tagof ab);
	#pragma unused abc
}

forward test_nosuggest5();
public test_nosuggest5()
{
	// As the name suggests, variable "staticvar" is defined as static
	// within another file, so the compiler shouldn't suggest its name here.
	return staticval;
}

forward test_nosuggest6();
public test_nosuggest6()
{
	// The compiler shouldn't suggest global variable "test_nosuggest6_val"
	// as it's not defined yet.
	return test_nosuggest6_val;
}
static test_nosuggest6_val;

forward test_nosuggest7();
public test_nosuggest7()
{
	// The compiler shouldn't try to suggest anything when tagof is used on
	// string/numeric literals.
	new a = tagof 0;
	new b = tagof "";
	return a + b;
}

forward test_e017();
public test_e017()
{
	// error 017: undefined symbol "val"; did you mean "var"?
	new var = 1;
	printf("%d\n", val);
	#pragma unused var

	// error 017: undefined symbol "celmax"; did you mean "cellmax"?
	printf("%d\n", celmax);

	// error 017: undefined symbol "strcaf"; did you mean "strcat"?
	new str[4] = "a";
	strcaf(str, "b");

	// error 017: undefined symbol "DoNothin"; did you mean "DoNothing"?
	DoNothin();

	// error 017: undefined symbol "test_e17"; did you mean "test_e017"?
	printf("%d\n", tagof test_e17);
}
DoNothing(){}
#pragma unused DoNothing

forward test_e019();
public test_e019()
{
	// error 019: not a label: "lb"; did you mean "lbl"?
lbl:
	goto lb;
}

forward test_e020();
public test_e020()
{
	// error 020: invalid symbol name "assert"; did you mean "asset"?
	new asset = 0;
	printf("%d\n", defined assert);
	#pragma unused asset
}

forward test_e080();
public test_e080()
{
	// error 080: unknown symbol, or not a constant symbol (symbol "idx"); did you mean "id"?
	new values[1];
	new idx = 0;
	const id = 0;
	printf("%d\n", sizeof values[idx]);
	#pragma unused values, idx, id
}

stock func1()<automaton_1:STATE_1>{}
stock func2()<automaton_2:BEING_1>{}

forward test_e086();
public test_e086()
{
	// error 086: unknown automaton "automaton1"; did you mean "automaton_1"?
	state automaton1:STATE_1;
}

forward test_e087();
public test_e087()
{
	// error 087: unknown state BEING1" for automaton "automaton_2"; did you mean "BEING_1"?
	state automaton_2:BEING1;

	// error 087: unknown state "STATE_1" for automaton "automaton_2"; did you mean "automaton_1:STATE_1"?
	state automaton_2:STATE_1;
}

new test_e017_sug2_var <automaton_3:STATE_2>;
forward test_e017_sug2();
public test_e017_sug2()
{
	printf("%d\n", test_e017_sug2_var); // error 017: undefined symbol "test_e017_sug2_var"; state variable out of scope
}
forward test_e017_sug2_func();
public test_e017_sug2_func() <automaton_3:STATE_1>
{
	printf("%d\n", test_e017_sug2_var); // error 017: undefined symbol "test_e017_sug2_var"; state variable out of scope
}
public test_e017_sug2_func() <automaton_3:STATE_2>
{
	#pragma unused test_e017_sug2_var
}
