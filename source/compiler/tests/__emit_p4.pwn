#include "__emit.inc"


stock test__data_offset(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit load.pri global_var;
	emit load.pri local_static_var;

	// should trigger an error
	emit load.pri global_const;
	emit load.pri global_func;
	emit load.pri local_refvar;
	emit load.pri local_refarray;
	emit load.pri local_const;
	emit load.pri local_var;
	emit load.pri local_label;
	emit load.pri 0;
}

stock test__local_var(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit load.s.pri global_const;
	emit load.s.pri local_refvar;
	emit load.s.pri local_refarray;
	emit load.s.pri local_const;
	emit load.s.pri local_var;
	emit load.s.pri 20;
	emit load.s.pri -20;
	emit stor.s.pri global_const;
	emit stor.s.pri local_const;
	emit stor.s.pri local_var;
	emit stor.s.pri 20;
	emit stor.s.pri -20;

	// should trigger an error
	emit load.s.pri global_var;
	emit load.s.pri global_func;
	emit load.s.pri local_static_var;
	emit load.s.pri local_label;
	emit stor.s.pri local_refvar;
	emit stor.s.pri local_refarray;
	emit load.s.pri -global_var;
	emit load.s.pri -global_func;
	emit load.s.pri -local_static_var;
	emit load.s.pri -local_label;
	emit stor.s.pri -local_refvar;
	emit stor.s.pri -local_refarray;
}


main()
{
	new t, a[1];
	test__data_offset(t, a); // 8
	test__local_var(t, a); // 6
}
