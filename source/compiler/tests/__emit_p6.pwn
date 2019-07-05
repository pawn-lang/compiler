#include "__emit.inc"


stock test__op_load_u_pri_alt(&local_refvar, const local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit load.u.pri global_const;
	emit load.u.pri global_var;
	emit load.u.pri local_const;
	emit load.u.pri local_var;
	emit load.u.pri local_static_var;
	emit load.u.pri local_refvar;
	emit load.u.pri global_const + local_const;
	emit load.u.pri global_var * local_var;
	emit load.u.pri local_refarray[0];

	// should trigger an error
	emit load.u.pri global_func;
	emit load.u.pri global_native;
	emit load.u.pri local_array;
	emit load.u.pri local_refarray;
}

stock test__op_stor_u_pri_alt(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit stor.u.pri global_var;
	emit stor.u.pri local_var;
	emit stor.u.pri local_static_var;
	emit stor.u.pri local_refvar;
	emit stor.u.pri local_array[1];
	emit stor.u.pri local_refarray[local_const];

	// should trigger an error
	emit stor.u.pri global_const;
	emit stor.u.pri global_func;
	emit stor.u.pri global_native;
	emit stor.u.pri global_const_var;
	emit stor.u.pri local_const;
	emit stor.u.pri local_array;
	emit stor.u.pri local_refarray;
}

stock test__op_addr_u_pri_alt(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit addr.u.pri global_var;
	emit addr.u.pri global_const_var;
	emit addr.u.pri local_var;
	emit addr.u.pri local_static_var;
	emit addr.u.pri local_refvar;
	emit addr.u.pri local_array[1];
	emit addr.u.pri local_refarray[local_const];

	// should trigger an error
	emit addr.u.pri global_const;
	emit addr.u.pri global_func;
	emit addr.u.pri global_native;
	emit addr.u.pri local_const;
	emit addr.u.pri local_array;
	emit addr.u.pri local_refarray;
}

stock test__push_u(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit push.u global_const;
	emit push.u global_var;
	emit push.u local_refvar;
	emit push.u local_const;
	emit push.u local_var;
	emit push.u local_static_var;
	emit push.u (global_const + local_const);
	emit push.u (global_var * local_var + local_static_var + local_refvar);
	emit push.u local_refarray[0];

	// should trigger an error
	emit push.u global_func;
	emit push.u global_native;
	emit push.u local_array;
	emit push.u local_refarray;
}


main()
{
	new t, a[2];
	test__op_load_u_pri_alt(t, a); // 4
	test__op_stor_u_pri_alt(t, a); // 7
	test__op_addr_u_pri_alt(t, a); // 6
	test__push_u(t, a); // 4
}
