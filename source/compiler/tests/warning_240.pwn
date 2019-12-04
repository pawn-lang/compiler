#include <core>
#include <console>

new global_var;
static global_static_var;

test_local()
{
	new local_var;
	local_var = 1;
	local_var = 2; // warning
	local_var = 3; // warning
	#pragma unused local_var
	local_var = 4;
	new local_var2 = 0;
	local_var2 = 1;
	#pragma unused local_var2
} // warning (value assigned to "local_var"
  // wasn't used upon symbol destruction)

test_local_static()
{
	static local_static_var = 0;
	local_static_var = 1; // warning
	local_static_var = 2; // warning
	#pragma unused local_static_var
	local_static_var = 4;
}

test_global()
{
	global_var = 1;
	global_var = 2;
	global_var = 3;
	#pragma unused global_var
	global_var = 4;
}

test_global_static()
{
	global_static_var = 1;
	global_static_var = 2;
	global_static_var = 3;
	#pragma unused global_static_var
	global_static_var = 4;
}

test_arg(arg)
{
	arg = 0;
	arg = 1; // warning (240, 204)
}

main()
{
	test_local();
	test_local_static();
	test_global();
	test_global_static();
	new x = 0;
	test_arg(x);
}
