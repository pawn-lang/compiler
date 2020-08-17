#include <core>
#include <console>

new g_var = 0;

test_if_1()
{
	if (g_var != 0)
		return 0;
	else
		return 1;
	return 2; // warning 225: unreachable code
}

test_if_2()
{
	new i = 0;
	while (i < 10)
	{
		if (g_var != 0)
			return;
		else
			continue;
		i++; // warning 225: unreachable code
	}
}

main()
{
	test_if_1();
	test_if_2();
}
