stock UseVariable(var) return var;

stock const bool:TRUE = true, bool:FALSE = false;

new global_var;
static global_static_var;

test_locals()
{
	{
		new local_var = 1; // warning 240
		local_var = 2;
		UseVariable(local_var);
		local_var = 3; // warning 204
	}

	{
		// Zero-initialization at definition is a special case, as it doesn't
		// generate any extra code, because variables are already zero-initialized
		// by default. Also, when storing a value into the variable, people often
		// forget that they zero-initialized the said variable at its definition
		// and didn't use that value, so usually warning 240 doesn't mean anything
		// suspicious in such cases, and thus can be just annoying to the user.
		// This is why if the variable was zero-initialized at definition,
		// warning 240 is not printed for the subsequent assignment.
		new local_var = 0;
		local_var = 1;
		UseVariable(local_var);
	}

	{
		new local_var = 1;
		if (TRUE)
		{
			local_var = 2; // warning 240
		}
		// The previous assignment ("local_var = 2") should be reported as unused.
		local_var = 3;
		UseVariable(local_var);
	}

	{
		new local_var;
		if (TRUE)
		{
			local_var = 1; // warning 240
		}
		else
		{
		}
		// The previous assignment ("local_var = 1") should be reported as unused.
		local_var = 2;
		UseVariable(local_var);
	}

	{
		new local_var;
		if (TRUE)
		{
		}
		else
		{
			local_var = 1; // warning 240
		}
		// The previous assignment ("local_var = 1") should be reported as unused.
		local_var = 2;
		UseVariable(local_var);
	}

	{
		new local_var = 1; // warning 240
		if (TRUE) {}
		// The previous assignment ("local_var = 1") should be reported as unused.
		local_var = 2;
		UseVariable(local_var);
	}

	{
		new local_var = 1;
		switch (TRUE)
		{
			default: local_var = 2; // warning 240
		}
		// The previous assignment ("local_var = 2") should be reported as unused.
		local_var = 3;
		UseVariable(local_var);
	}

	{
		new local_var = 1;
		switch (TRUE)
		{
			case true: local_var = 2; // warning 240
			default: {}
		}
		// The previous assignment ("local_var = 2") should be reported as unused.
		local_var = 3;
		UseVariable(local_var);
	}

	{
		new local_var = 1;
		switch (TRUE)
		{
			case true: {}
			default: local_var = 2; // warning 240
		}
		// The previous assignment ("local_var = 2") should be reported as unused.
		local_var = 3;
		UseVariable(local_var);
	}

	{
		new local_var = 1; // warning 240
		switch (TRUE)
		{
			default: {}
		}
		// The previous assignment ("local_var = 1") should be reported as unused.
		local_var = 2;
		UseVariable(local_var);
	}

	{
		new local_var;
		do {
			local_var = 0;
		} while (FALSE);
		// The previous assignment ("local_var = 0") should NOT be reported as unused,
		// as the compiler can't analyze the control flow of the loop.
		local_var = 1;
		UseVariable(local_var);
	}

	{
		new local_var = 1; // warning 240
		do {} while (FALSE);
		// The previous assignment ("local_var = 1") should be reported as unused.
		local_var = 1;
		UseVariable(local_var);
	}

	{
		// "warning 204" is not applicable to "static" local variables, as the
		// assigned value can be used on the next function call, which is why
		// assignment "local_static_var = 4" should NOT be reported as unused.
		static local_static_var = 1;
		local_static_var = 1;
		UseVariable(local_static_var);
		local_static_var = 4;
	}
}

test_globals()
{
	// Assignments to global variables are not tracked
	global_var = 1;
	global_var = 2;
	UseVariable(global_var);
	global_var = 3;

	global_static_var = 1;
	global_static_var = 2;
	UseVariable(global_static_var);
	global_static_var = 3;
}

test_args(arg, &refarg) // warning 240 (symbol "arg")
{
	// Technically function arguments are like local variables, except that they
	// have a value implicitly assigned to them at the start of the function body.
	// This is why on the subsequent assignment ("arg = 1") the compiler should warn
	// about the previously assigned value being unused.
	arg = 1;
	if (TRUE)
		arg = 2; // warning 240
	do {} while (FALSE);
	arg = 3; // warning 204

	// "warning 203" is not applicable to references, as the value might be used
	// outside of the function, but warning 240 should still work for them.
	refarg = 1; // warning 240
	refarg = 2;
	UseVariable(refarg);
	refarg = 3;
}

test_goto()
{
	{
		new local_var = 0;
		UseVariable(local_var);
lbl_1:
		if (TRUE)
		{
			// Because of "goto", the compiler can't be certain if assignment
			// "local_var = 1" is used or not. Since the jump is being made to
			// a previously defined label, the compiler assumes the assignment
			// is used, to avoid false-positives.
			local_var = 1;
			goto lbl_1;
		}
	}
	{
		new local_var = 0;
		UseVariable(local_var);
		if (TRUE)
		{
			// Since the jump is being made to a yet undefined label, and the assignment
			// is not used at any later point, this means the compiler may continue working
			// in its usual (linear) way to see if assignment "local_var2 = 1" is used or not.
			local_var = 1; // warning 204
			goto lbl_2;
		}
lbl_2:
	}
	{
		for (new x = 5; --x != 0;)
		{
			if (x == 2)
			{
				x = 1;
				// The "x = 3;" line is used, as the target label of "goto"
				// is located inside of the current loop, so this shouldn't
				// trigger warning 240.
				goto lbl_3;
			}
			x = 3;
		lbl_3:
		}
	}
}

stock Tag:operator =(oper)
	return Tag:oper;

test_overloaded()
{
	// Overloaded assignments are essentially function calls which may have
	// desirable side effects, so they shouldn't trigger warning 204.
	new Tag:a = 1;
	new Tag:b;
	b = 2;
}

main()
{
	test_locals();
	test_globals();
	new x;
	test_args(0,x);
	test_goto();
	test_overloaded();
}
