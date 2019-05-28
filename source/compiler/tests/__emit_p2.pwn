#include "__emit.inc"


stock test__shift(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit shl.c.pri global_const;
	emit shl.c.pri local_const;
	emit shl.c.pri 0;
	emit shl.c.pri 1;
	emit shl.c.pri 15;
	emit shl.c.pri (cellbits / charbits);
#if cellbits == 16
	emit shl.c.pri 15;
	emit shl.c.pri 0xF;
#elseif cellbits == 32
	emit shl.c.pri 31;
	emit shl.c.pri 0x1F;
#else // cellbits == 64
	emit shl.c.pri 63;
	emit shl.c.pri 0x3F;
#endif

	// should trigger an error
	emit shl.c.pri global_var;
	emit shl.c.pri global_func;
	emit shl.c.pri local_refvar;
	emit shl.c.pri local_refarray;
	emit shl.c.pri local_var;
	emit shl.c.pri local_static_var;
	emit shl.c.pri local_label;
	emit shl.c.pri -1;
	emit shl.c.pri -0x1;
#if cellbits == 16
	emit shl.c.pri 16;
	emit shl.c.pri 0x10;
#elseif cellbits == 32
	emit shl.c.pri 32;
	emit shl.c.pri 0x20;
#else // cellbits == 64
	emit shl.c.pri 64;
	emit shl.c.pri 0x40;
#endif
}

stock test__label(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit jump local_label;
	emit jump local_label2;
	emit jump :local_label2;
local_label2:

	// should trigger an error
	emit jump global_const;
	emit jump global_var;
	emit jump global_func;
	emit jump local_refvar;
	emit jump local_refarray;
	emit jump local_const;
	emit jump local_var;
	emit jump local_static_var;
	emit jump 0;
}


main()
{
	new t, a[1];
	test__shift(t, a); // 11
	test__label(t, a); // 9
}
