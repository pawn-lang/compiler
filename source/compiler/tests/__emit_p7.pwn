#include "__emit.inc"


stock test__push_u_adr(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit push.u.adr global_var;
	emit push.u.adr global_const_var;
	emit push.u.adr local_refvar;
	emit push.u.adr local_var;
	emit push.u.adr local_static_var;
	emit push.u.adr local_array[1];
	emit push.u.adr local_refarray[local_const];

	// should trigger an error
	emit push.u.adr global_const;
	emit push.u.adr global_func;
	emit push.u.adr global_native;
	emit push.u.adr local_const;
	emit push.u.adr local_array;
	emit push.u.adr local_refarray;
	emit push.u.adr local_array{1};
	emit push.u.adr local_array{local_var};
}

stock test__zero_u(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit zero.u global_var;
	emit zero.u local_refvar;
	emit zero.u local_var;
	emit zero.u local_static_var;
	emit zero.u local_array[1];
	emit zero.u local_refarray[local_const];

	// should trigger an error
	emit zero.u global_const;
	emit zero.u global_func;
	emit zero.u global_native;
	emit zero.u global_const_var;
	emit zero.u local_const;
	emit zero.u local_array;
	emit zero.u local_refarray;
}

stock test__inc_dec_u(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	emit inc.u global_var;
	emit inc.u local_refvar;
	emit inc.u local_var;
	emit inc.u local_static_var;
	emit inc.u local_array[1];
	emit inc.u local_refarray[local_const];

	// should trigger an error
	emit inc.u global_const;
	emit inc.u global_func;
	emit inc.u global_native;
	emit inc.u global_const_var;
	emit inc.u local_const;
	emit inc.u local_array;
	emit inc.u local_refarray;
}


main()
{
	new t, a[2];
	test__push_u_adr(t, a); // 8
	test__zero_u(t, a); // 7
	test__inc_dec_u(t, a); // 7
}
