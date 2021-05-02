// This test makes sure overloaded operators for tag `bool:` don't trigger
// warning 247. The code in 'main()' is copied from file "warning_247.pwn",
// but with the lines that test operators `<<`, `>>`, `>>>` and `~` removed,
// as shift operators can't be overloaded and `~` is reserved for destructors.

bool:operator -(bool:oper) return oper;
bool:operator ++(bool:oper) return oper;
bool:operator --(bool:oper) return oper;
bool:operator *(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator /(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator %(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator +(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator -(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator <=(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator >=(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator <(bool:oper1, bool:oper2) return oper1,oper2;
bool:operator >(bool:oper1, bool:oper2) return oper1,oper2;

main()
{
	new bool:a = true, bool:b = false;
	if (a <= b) {}
	if (a >= b) {}
	if (a < b) {}
	if (a > b) {}
	if (-a) {}
	if (++a) {}
	if (a++) {}
	if (--a) {}
	if (a--) {}
	if (a * b) {}
	if (a / b) {}
	if (a % b) {}
	if (a + b) {}
	if (a - b) {}
}
