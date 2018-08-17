#include <console>
#include <string>
main()
{
	new var_01 = 0;		// the compiler shouldn't suggest any name for this error
	printf("%d", var);	// since "var_01" and "var" differ by more than 2 symbols

	new var = 1;
	printf("%d", val); // error 017: undefined symbol "val"; did you mean "var"?

	printf("%d", celmax); // error 017: undefined symbol "celmax"; did you mean "cellmax"?

	new str[4] = "a";
	strcaf(str, "b"); // error 017: undefined symbol "strcaf"; did you mean "strcat"?

lbl:
	goto lb; // error 019: not a label: "lb"; did you mean "lbl"?

	// error 086: unknown automaton "dialpg"; did you mean "dialog"?
	state dialpg:DLG_REGISTER;

	// error 087: unknown state "DLGLOGIN" for automaton "dialog"; did you mean "DLG_LOGIN"?
	state dialog:DLGLOGIN;
}
stock DialogHandler() <dialog:DLG_REGISTER> {}
stock DialogHandler() <dialog:DLG_LOGIN> {}
