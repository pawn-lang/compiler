#include <console>

const func1_addr = __addressof(Func1); // error

Func1() {}
Func2() {}
Func3() {}

new const functions[3] =
{
	__addressof(Func1),
	__addressof(Func2),
	__addressof(Func3)
};

#if __addressof Func1 > 0 // error
#endif

main()
{
	const a = __addressof func1_addr; // error
	#pragma unused a

	const b = __addressof printf; // error
	#pragma unused b

lbl:
	const c = __addressof lbl;
	#pragma unused c

	const d = __addressof functions;
	#pragma unused d

	const e = __addressof functions[1];
	#pragma unused e

	static x = 0;
	const f = __addressof x;
	#pragma unused f

	static arr1[3][2] = { { 0, ... }, ... };
	const g = __addressof arr1[2][1];
	#pragma unused g

	new y = 0;
	new const h = __addressof y;
	#pragma unused h

	new arr2[3][2] = { { 0, ... }, ... };
	new const i = __addressof arr2;
	#pragma unused i
}
