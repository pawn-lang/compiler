#include "__emit.inc"


stock test__function(&local_refvar, local_refarray[])
{
	const local_const = 0;
	new local_var = 0;
	static local_static_var = 0;
local_label:

	// ok
	emit call global_func;
	emit sysreq.c global_native;
	emit sysreq.n global_native 0;

	// should trigger an error
	emit call global_const;
	emit call global_var;
	emit call local_refvar;
	emit call local_refarray;
	emit call local_const;
	emit call local_var;
	emit call local_static_var;
	emit call local_label;
	emit call 0;
	emit sysreq.c global_const;
	emit sysreq.c global_var;
	emit sysreq.c local_refvar;
	emit sysreq.c local_refarray;
	emit sysreq.c local_const;
	emit sysreq.c local_var;
	emit sysreq.c local_static_var;
	emit sysreq.c local_label;
	emit sysreq.c 0;
}


main()
{
	new t, a[1];
	test__function(t, a); // 18
}
