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
	emit lodb.i 1;
	emit lodb.i 2;
	emit lodb.i 4;
	emit lodb.i 0x1;
	emit lodb.i 0x2;
	emit lodb.i 0x4;
	emit lodb.i global_const_1;
	emit lodb.i local_const_1;

	// should trigger an error
	emit lodb.i global_const;
	emit lodb.i global_var;
	emit lodb.i global_func;
	emit lodb.i local_refvar;
	emit lodb.i local_const;
	emit lodb.i local_var;
	emit lodb.i local_static_var;
	emit lodb.i local_label;
	emit lodb.i -1;
	emit lodb.i 3;
	emit lodb.i 5;
	emit lodb.i -0x1;
	emit lodb.i 0x3;
	emit lodb.i 0x5;
}

stock test__op_align(&local_refvar)
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit align.pri global_const;
	emit align.pri local_const;
	emit align.pri 0;
	emit align.pri 1;
	emit align.pri 2;
	emit align.pri 3;
	emit align.pri 0x0;
	emit align.pri 0x1;
	emit align.pri 0x2;
	emit align.pri 0x3;

	// should trigger an error
	emit align.pri global_var;
	emit align.pri global_func;
	emit align.pri local_refvar;
	emit align.pri local_var;
	emit align.pri local_static_var;
	emit align.pri local_label;
	emit align.pri -1;
	emit align.pri 4;
	emit align.pri -0x1;
	emit align.pri 0x4;
}


main()
{
	new t;
	test__op_lodb_strb(t); // 14
	test__op_align(t); // 10
}
