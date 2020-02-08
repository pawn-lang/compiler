#include "__emit.inc"


stock test__push_u_adr(&local_refvar, local_refarray[])
{
	const local_const = 1;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit push.u.adr global_var;
	__emit push.u.adr global_const_var;
	__emit push.u.adr local_refvar;
	__emit push.u.adr local_var;
	__emit push.u.adr local_static_var;
	__emit push.u.adr local_array[1];
	__emit push.u.adr local_refarray[local_const];

	// should trigger an error
	__emit push.u.adr global_const;
	__emit push.u.adr global_func;
	__emit push.u.adr global_native;
	__emit push.u.adr local_const;
	__emit push.u.adr local_array;
	__emit push.u.adr local_refarray;
	__emit push.u.adr local_array{1};
	__emit push.u.adr local_array{local_var};
}

stock test__zero_u(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit zero.u global_var;
	__emit zero.u local_refvar;
	__emit zero.u local_var;
	__emit zero.u local_static_var;
	__emit zero.u local_array[1];
	__emit zero.u local_refarray[local_const];

	// should trigger an error
	__emit zero.u global_const;
	__emit zero.u global_func;
	__emit zero.u global_native;
	__emit zero.u global_const_var;
	__emit zero.u local_const;
	__emit zero.u local_array;
	__emit zero.u local_refarray;
}

stock test__inc_dec_u(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
	new local_array[2];

	// ok
	__emit inc.u global_var;
	__emit inc.u local_refvar;
	__emit inc.u local_var;
	__emit inc.u local_static_var;
	__emit inc.u local_array[1];
	__emit inc.u local_refarray[local_const];

	// should trigger an error
	__emit inc.u global_const;
	__emit inc.u global_func;
	__emit inc.u global_native;
	__emit inc.u global_const_var;
	__emit inc.u local_const;
	__emit inc.u local_array;
	__emit inc.u local_refarray;
}


main()
{
	new t, a[2];
	test__push_u_adr(t, a); // 8
	test__zero_u(t, a); // 7
	test__inc_dec_u(t, a); // 7
}
