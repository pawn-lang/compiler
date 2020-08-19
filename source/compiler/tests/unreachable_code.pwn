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

test_if_3()
{
	if (g_var != 0)
		return 0;
	else
		return 1;
	// shouldn't cause warning 209 ("function should return a value")
}

test_if_4()
{
	if (g_var != 0)
		return 0;
	else
		for (;;) {}
	// shouldn't cause warning 209 ("function should return a value")
}

test_if_5()
{
	if (g_var != 0)
		return 1;
	else
		{}
	return 1;
}

test_switch_1()
{
	new var0 = 0, var1 = 1, var2 = 2, var3 = 3;
	switch (g_var)
	{
		case 0:
			return var0;
		case 1:
			return var1;
		default:
			return var2;
	}
	return var3; // warning 225: unreachable code
}

test_switch_2()
{
	new i = 0;
	while (i < 10)
	{
		switch (g_var)
		{
			case 0:
				break;
			default:
				continue;
		}
		i++; // warning 225: unreachable code
	}
	return i;
}

test_switch_3()
{
	switch (g_var)
	{
		case 0:
			return 0;
		case 1:
			return 1;
	}
	// shouldn't cause warning 225 ("unreachable code")
	return 2;
}

test_switch_4()
{
	switch (g_var)
	{
		case 0:
			return 0;
		case 1:
			return 1;
		default:
			return 2;
	}
	// shouldn't cause warning 225 ("unreachable code")
	return 3;
}

test_endless()
{
	if (g_var != 0)
		return 0;
	for (;;) {}
	// shouldn't cause warning 209 ("function should return a value")
}

test_goto_1()
{
label_1:
	goto label_1;
	return 0; // warning 225: unreachable code
}

test_goto_2()
{
	if (g_var == 0)
		goto label_2;
label_1:
	goto label_1;
label_2:
	// shouldn't cause warning 225 ("unreachable code")
	return 1;
}

test_goto_3()
{
	if (g_var != 0)
		return 1;
label_1:
	goto label_1;
	// shouldn't cause warning 209 ("function should return a value")
}

test_goto_4()
{
	new var = 0;
	if (var == 0)
		goto label_1;
	if (var != 0)
		return 0;
	else
		return 1;
	// shouldn't cause warning 225 ("unreachable code")
label_1:
	return 2;
}

test_exit()
{
	exit 0;
	return 0; // warning 225: unreachable code
}

main()
{
	test_if_1();
	test_if_2();
	test_if_3();
	test_if_4();
	test_if_5();
	test_switch_1();
	test_switch_2();
	test_switch_3();
	test_switch_4();
	test_endless();
	test_goto_1();
	test_goto_2();
	test_goto_3();
	test_goto_4();
	test_exit();
}
