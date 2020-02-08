#include "__emit.inc"


stock test__any(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit const.pri global_const;
	__emit const.pri global_var;
	__emit const.pri global_func;
	__emit const.pri local_refvar;
	__emit const.pri local_refarray;
	__emit const.pri local_const;
	__emit const.pri local_var;
	__emit const.pri local_static_var;
	__emit const.pri local_label;
	__emit const.pri :local_label;
	__emit const.pri :local_label2;
	__emit const.pri 0;
	__emit const.pri -0;
	__emit const.pri 1;
	__emit const.pri -1;
	__emit const.pri 0x0;
	__emit const.pri -0x0;
	__emit const.pri 0x1;
	__emit const.pri -0x1;
#if cellbits == 16
	__emit const.pri 32767;
	__emit const.pri -32768;
	__emit const.pri 0x7FFF;
	__emit const.pri -0x7FFF;
	__emit const.pri 0x8000;
	__emit const.pri -0x8000;
#elseif cellbits == 32
	__emit const.pri 2147483647;
	__emit const.pri -2147483648;
	__emit const.pri 0x7FFFFFFF;
	__emit const.pri -0x7FFFFFFF;
	__emit const.pri 0x80000000;
	__emit const.pri -0x80000000;
#else // cellbits == 64
	__emit const.pri 9223372036854775807;
	__emit const.pri -9223372036854775808;
	__emit const.pri 0x7FFFFFFFFFFFFFFF;
	__emit const.pri -0x7FFFFFFFFFFFFFFF;
	__emit const.pri 0x8000000000000000;
	__emit const.pri -0x8000000000000000;
#endif
	__emit const.pri (cellbits / charbits * 2);

	// should trigger an error
	__emit const.pri -global_func;
	__emit const.pri -local_label;
	__emit const.pri --0;
	__emit const.pri local_label2;
	__emit const.pri -:local_label;

local_label2:
}

stock test__integer(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit jrel global_const;
	__emit jrel local_const;
	__emit jrel 0;
	__emit jrel 1;
	__emit jrel -1;
	__emit jrel 0x1;
	__emit jrel -0x1;
	__emit jrel (cellbits / charbits * 2);

	// should trigger an error
	__emit jrel global_var;
	__emit jrel global_func;
	__emit jrel local_refvar;
	__emit jrel local_refarray;
	__emit jrel local_var;
	__emit jrel local_static_var;
	__emit jrel local_label;
}

stock test__nonneg(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit cmps global_const;
	__emit cmps local_const;
	__emit cmps 0;
	__emit cmps 1;
	__emit cmps 0x1;
	__emit cmps (cellbits / charbits * 2);
#if cellbits == 16
	__emit cmps 0x7FFF;
#elseif cellbits == 32
	__emit cmps 0x7FFFFFFF;
#else // cellbits == 64
	__emit cmps 0x7FFFFFFFFFFFFFFF;
#endif

	// should trigger an error
	__emit cmps global_var;
	__emit cmps global_func;
	__emit cmps local_refvar;
	__emit cmps local_refarray;
	__emit cmps local_var;
	__emit cmps local_static_var;
	__emit cmps local_label;
	__emit cmps -1;
	__emit cmps -0x1;
#if cellbits == 16
	__emit cmps 0x8000;
#elseif cellbits == 32
	__emit cmps 0x80000000;
#else // cellbits == 64
	__emit cmps 0x8000000000000000;
#endif
}


main()
{
	new t;
	static a[1];
	test__any(t, a); // 4
	test__integer(t, a); // 7
	test__nonneg(t, a); // 10
}
