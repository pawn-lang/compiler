#include "__emit.inc"


stock test__any(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit const.pri global_const;
	emit const.pri global_var;
	emit const.pri global_func;
	emit const.pri local_refvar;
	emit const.pri local_refarray;
	emit const.pri local_const;
	emit const.pri local_var;
	emit const.pri local_static_var;
	emit const.pri local_label;
	emit const.pri :local_label;
	emit const.pri :local_label2;
	emit const.pri 0;
	emit const.pri -0;
	emit const.pri 1;
	emit const.pri -1;
	emit const.pri 0x0;
	emit const.pri -0x0;
	emit const.pri 0x1;
	emit const.pri -0x1;
#if cellbits == 16
	emit const.pri 32767;
	emit const.pri -32768;
	emit const.pri 0x7FFF;
	emit const.pri -0x7FFF;
	emit const.pri 0x8000;
	emit const.pri -0x8000;
#elseif cellbits == 32
	emit const.pri 2147483647;
	emit const.pri -2147483648;
	emit const.pri 0x7FFFFFFF;
	emit const.pri -0x7FFFFFFF;
	emit const.pri 0x80000000;
	emit const.pri -0x80000000;
#else // cellbits == 64
	emit const.pri 9223372036854775807;
	emit const.pri -9223372036854775808;
	emit const.pri 0x7FFFFFFFFFFFFFFF;
	emit const.pri -0x7FFFFFFFFFFFFFFF;
	emit const.pri 0x8000000000000000;
	emit const.pri -0x8000000000000000;
#endif
	emit const.pri (cellbits / charbits * 2);

	// should trigger an error
	emit const.pri -global_func;
	emit const.pri -local_label;
	emit const.pri --0;
	emit const.pri local_label2;
	emit const.pri -:local_label;

local_label2:
}

stock test__integer(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit jrel global_const;
	emit jrel local_const;
	emit jrel 0;
	emit jrel 1;
	emit jrel -1;
	emit jrel 0x1;
	emit jrel -0x1;
	emit jrel (cellbits / charbits * 2);

	// should trigger an error
	emit jrel global_var;
	emit jrel global_func;
	emit jrel local_refvar;
	emit jrel local_refarray;
	emit jrel local_var;
	emit jrel local_static_var;
	emit jrel local_label;
}

stock test__nonneg(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit cmps global_const;
	emit cmps local_const;
	emit cmps 0;
	emit cmps 1;
	emit cmps 0x1;
	emit cmps (cellbits / charbits * 2);
#if cellbits == 16
	emit cmps 0x7FFF;
#elseif cellbits == 32
	emit cmps 0x7FFFFFFF;
#else // cellbits == 64
	emit cmps 0x7FFFFFFFFFFFFFFF;
#endif

	// should trigger an error
	emit cmps global_var;
	emit cmps global_func;
	emit cmps local_refvar;
	emit cmps local_refarray;
	emit cmps local_var;
	emit cmps local_static_var;
	emit cmps local_label;
	emit cmps -1;
	emit cmps -0x1;
#if cellbits == 16
	emit cmps 0x8000;
#elseif cellbits == 32
	emit cmps 0x80000000;
#else // cellbits == 64
	emit cmps 0x8000000000000000;
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
