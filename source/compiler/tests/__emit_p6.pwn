#include "__emit.inc"


stock test__op_load_u_pri_alt(&local_refvar, const local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit load.u.pri global_const;
	__emit load.u.pri global_var;
	__emit load.u.pri local_const;
	__emit load.u.pri local_var;
	__emit load.u.pri local_static_var;
	__emit load.u.pri local_refvar;
	__emit load.u.pri global_const + local_const;
	__emit load.u.pri global_var * local_var;
	__emit load.u.pri local_refarray[0];

	// should trigger an error
	__emit load.u.pri global_func;
	__emit load.u.pri global_native;
	__emit load.u.pri local_array;
	__emit load.u.pri local_refarray;
}

stock test__op_stor_u_pri_alt(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit stor.u.pri global_var;
	__emit stor.u.pri local_var;
	__emit stor.u.pri local_static_var;
	__emit stor.u.pri local_refvar;
	__emit stor.u.pri local_array[1];
	__emit stor.u.pri local_refarray[local_const];

	// should trigger an error
	__emit stor.u.pri global_const;
	__emit stor.u.pri global_func;
	__emit stor.u.pri global_native;
	__emit stor.u.pri global_const_var;
	__emit stor.u.pri local_const;
	__emit stor.u.pri local_array;
	__emit stor.u.pri local_refarray;
}

stock test__op_addr_u_pri_alt(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit addr.u.pri global_var;
	__emit addr.u.pri global_const_var;
	__emit addr.u.pri local_var;
	__emit addr.u.pri local_static_var;
	__emit addr.u.pri local_refvar;
	__emit addr.u.pri local_array[1];
	__emit addr.u.pri local_refarray[local_const];

	// should trigger an error
	__emit addr.u.pri global_const;
	__emit addr.u.pri global_func;
	__emit addr.u.pri global_native;
	__emit addr.u.pri local_const;
	__emit addr.u.pri local_array;
	__emit addr.u.pri local_refarray;
}

stock test__push_u(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit push.u global_const;
	__emit push.u global_var;
	__emit push.u local_refvar;
	__emit push.u local_const;
	__emit push.u local_var;
	__emit push.u local_static_var;
	__emit push.u (global_const + local_const);
	__emit push.u (global_var * local_var + local_static_var + local_refvar);
	__emit push.u local_refarray[0];

	// should trigger an error
	__emit push.u global_func;
	__emit push.u global_native;
	__emit push.u local_array;
	__emit push.u local_refarray;
}


main()
{
	new t, a[2];
	test__op_load_u_pri_alt(t, a); // 4
	test__op_stor_u_pri_alt(t, a); // 7
	test__op_addr_u_pri_alt(t, a); // 6
	test__push_u(t, a); // 4
}
