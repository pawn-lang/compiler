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
	printf("x: %d\n", local_var);
	local_var = 4;
} // warning (value assigned to "local_var"
  // wasn't used upon symbol destruction)

test_local_static()
{
	static local_static_var;
	local_static_var = 1;
	local_static_var = 2; // warning
	local_static_var = 3; // warning
	printf("x: %d\n", local_static_var);
	local_static_var = 4;
}

test_global()
{
	global_var = 1;
	global_var = 2;
	global_var = 3;
	printf("x: %d\n", global_var);
	global_var = 4;
}

test_global_static()
{
	global_static_var = 1;
	global_static_var = 2;
	global_static_var = 3;
	printf("x: %d\n", global_static_var);
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
