#include "__emit.inc"


stock test__data_offset(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit load.pri global_var;
	__emit load.pri local_static_var;

	// should trigger an error
	__emit load.pri global_const;
	__emit load.pri global_func;
	__emit load.pri local_refvar;
	__emit load.pri local_refarray;
	__emit load.pri local_const;
	__emit load.pri local_var;
	__emit load.pri local_label;
	__emit load.pri 0;
}

stock test__local_var(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit load.s.pri global_const;
	__emit load.s.pri local_refvar;
	__emit load.s.pri local_refarray;
	__emit load.s.pri local_const;
	__emit load.s.pri local_var;
	__emit load.s.pri 20;
	__emit load.s.pri -20;
	__emit stor.s.pri global_const;
	__emit stor.s.pri local_const;
	__emit stor.s.pri local_var;
	__emit stor.s.pri 20;
	__emit stor.s.pri -20;

	// should trigger an error
	__emit load.s.pri global_var;
	__emit load.s.pri global_func;
	__emit load.s.pri local_static_var;
	__emit load.s.pri local_label;
	__emit stor.s.pri local_refvar;
	__emit stor.s.pri local_refarray;
	__emit load.s.pri -global_var;
	__emit load.s.pri -global_func;
	__emit load.s.pri -local_static_var;
	__emit load.s.pri -local_label;
	__emit stor.s.pri -local_refvar;
	__emit stor.s.pri -local_refarray;
}


main()
{
	new t, a[1];
	test__data_offset(t, a); // 8
	test__local_var(t, a); // 6
}
