#include "__emit.inc"


stock test__function(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	__emit call global_func;
	__emit sysreq.c global_native;
	__emit sysreq.n global_native 0;

	// should trigger an error
	__emit call global_const;
	__emit call global_var;
	__emit call local_refvar;
	__emit call local_refarray;
	__emit call local_const;
	__emit call local_var;
	__emit call local_static_var;
	__emit call local_label;
	__emit call 0;
	__emit sysreq.c global_const;
	__emit sysreq.c global_var;
	__emit sysreq.c local_refvar;
	__emit sysreq.c local_refarray;
	__emit sysreq.c local_const;
	__emit sysreq.c local_var;
	__emit sysreq.c local_static_var;
	__emit sysreq.c local_label;
	__emit sysreq.c 0;
}


main()
{
	new t, a[1];
	test__function(t, a); // 18
}
