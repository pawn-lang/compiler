#include "__emit.inc"


stock test__shift(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit shl.c.pri global_const;
	__emit shl.c.pri local_const;
	__emit shl.c.pri 0;
	__emit shl.c.pri 1;
	__emit shl.c.pri 15;
	__emit shl.c.pri (cellbits / charbits);
#if cellbits == 16
	__emit shl.c.pri 15;
	__emit shl.c.pri 0xF;
#elseif cellbits == 32
	__emit shl.c.pri 31;
	__emit shl.c.pri 0x1F;
#else // cellbits == 64
	__emit shl.c.pri 63;
	__emit shl.c.pri 0x3F;
#endif

	// should trigger an error
	__emit shl.c.pri global_var;
	__emit shl.c.pri global_func;
	__emit shl.c.pri local_refvar;
	__emit shl.c.pri local_refarray;
	__emit shl.c.pri local_var;
	__emit shl.c.pri local_static_var;
	__emit shl.c.pri local_label;
	__emit shl.c.pri -1;
	__emit shl.c.pri -0x1;
#if cellbits == 16
	__emit shl.c.pri 16;
	__emit shl.c.pri 0x10;
#elseif cellbits == 32
	__emit shl.c.pri 32;
	__emit shl.c.pri 0x20;
#else // cellbits == 64
	__emit shl.c.pri 64;
	__emit shl.c.pri 0x40;
#endif
}

stock test__label(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit jump local_label;
	__emit jump local_label2;
	__emit jump :local_label2;
local_label2:

	// should trigger an error
	__emit jump global_const;
	__emit jump global_var;
	__emit jump global_func;
	__emit jump local_refvar;
	__emit jump local_refarray;
	__emit jump local_const;
	__emit jump local_var;
	__emit jump local_static_var;
	__emit jump 0;
}


main()
{
	new t, a[1];
	test__shift(t, a); // 11
	test__label(t, a); // 9
}
