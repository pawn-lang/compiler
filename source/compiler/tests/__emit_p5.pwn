#include "__emit.inc"

const global_const_1 = 1;


stock test__op_lodb_strb(&local_refvar)
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:
	const local_const_1 = 0x1;

	// ok
	__emit lodb.i 1;
	__emit lodb.i 2;
	__emit lodb.i 4;
	__emit lodb.i 0x1;
	__emit lodb.i 0x2;
	__emit lodb.i 0x4;
	__emit lodb.i global_const_1;
	__emit lodb.i local_const_1;

	// should trigger an error
	__emit lodb.i global_const;
	__emit lodb.i global_var;
	__emit lodb.i global_func;
	__emit lodb.i local_refvar;
	__emit lodb.i local_const;
	__emit lodb.i local_var;
	__emit lodb.i local_static_var;
	__emit lodb.i local_label;
	__emit lodb.i -1;
	__emit lodb.i 3;
	__emit lodb.i 5;
	__emit lodb.i -0x1;
	__emit lodb.i 0x3;
	__emit lodb.i 0x5;
}

stock test__op_align(&local_refvar)
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit align.pri global_const;
	__emit align.pri local_const;
	__emit align.pri 0;
	__emit align.pri 1;
	__emit align.pri 2;
	__emit align.pri 3;
	__emit align.pri 0x0;
	__emit align.pri 0x1;
	__emit align.pri 0x2;
	__emit align.pri 0x3;

	// should trigger an error
	__emit align.pri global_var;
	__emit align.pri global_func;
	__emit align.pri local_refvar;
	__emit align.pri local_var;
	__emit align.pri local_static_var;
	__emit align.pri local_label;
	__emit align.pri -1;
	__emit align.pri 4;
	__emit align.pri -0x1;
	__emit align.pri 0x4;
}


main()
{
	new t;
	test__op_lodb_strb(t); // 14
	test__op_align(t); // 10
}
