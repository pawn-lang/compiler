stock UseVariable(arg)
{
	#pragma unused arg
}

stock UseVariableRef(&arg)
{
	#pragma unused arg
}

stock Use1DArray(const arg[])
{
	#pragma unused arg
}

stock Use1DArrayConst(const arg[])
{
	#pragma unused arg
}

stock Use2DArray(const arg[][])
{
	#pragma unused arg
}


new global_var1 = 0;
new global_var2;
new global_var3;
new global_var4;
new global_var5;
new global_arr1d1[] = "hi there!";
new global_arr1d2[8];
new global_arr1d3[8];
new global_arr1d4[8];
new global_arr1d5[8];
new global_arr1d6[8];
new global_arr2d1[2][8] = { { 0 }, ... };
new global_arr2d2[2][8];
new global_arr2d3[2][8];
new global_arr2d4[2][8];
new global_arr2d5[2][8];
new global_arr2d6[2][8];

TestFunc(arg_var, const arg_arr1d[], const arg_arr2d[][])
{
	UseVariable(arg_var);
	Use1DArray(arg_arr1d);
	Use2DArray(arg_arr2d);

	new local_var1 = 0;
	UseVariable(local_var1);

	new local_var2;
	local_var2 = 0;
	UseVariable(local_var2);

	new local_var3;
	UseVariable(local_var3); // warning

	new local_var4;
	local_var4 = local_var4 + 1; // warning
	UseVariable(local_var4);

	new local_var5;
	++local_var5;
	UseVariable(local_var5);

	new local_var6;
	local_var6++;
	UseVariable(local_var6);

	new local_arr1d1[] = "hi there!";
	Use1DArray(local_arr1d1);

	new local_arr1d2[8];
	local_arr1d2 = "oh, hi!";
	Use1DArray(local_arr1d2);

	new local_arr1d3[8];
	local_arr1d3[0] = 1;
	Use1DArray(local_arr1d3);

	new local_arr1d4[8];
	Use1DArray(local_arr1d4); // warning

	new local_arr1d5[8];
	local_arr1d5[0] = local_arr1d5[0] + 1; // warning
	Use1DArray(local_arr1d5);

	new local_arr1d6[8];
	local_arr1d6[0] = local_arr1d6[1]; // warning
	Use1DArray(local_arr1d6);

	new local_arr1d7[8];
	local_arr1d7[local_arr1d7[0]] = 1; // warning
	Use1DArray(local_arr1d7);

	new local_arr1d8[8];
	++local_arr1d8[0];
	Use1DArray(local_arr1d8);

	new local_arr1d9[8];
	local_arr1d9[0]++;
	Use1DArray(local_arr1d9);

	new local_arr2d1[2][8] = { { 0 }, ... };
	Use2DArray(local_arr2d1);

	new local_arr2d2[2][8];
	local_arr2d2[0] = "oh, hi!";
	Use2DArray(local_arr2d2);

	new local_arr2d3[2][8];
	local_arr2d3[0][0] = 1;
	Use2DArray(local_arr2d3);

	new local_arr2d4[2][8];
	Use2DArray(local_arr2d4); // warning

	new local_arr2d5[2][8];
	local_arr2d5[0][0] = local_arr2d5[0][0] + 1; // warning
	Use2DArray(local_arr2d5);

	new local_arr2d6[2][8];
	local_arr2d6[0][0] = local_arr2d6[1][0]; // warning
	Use2DArray(local_arr2d6);

	new local_arr2d7[2][8];
	local_arr2d7[0][local_arr2d7[0][0]] = 0; // warning
	Use2DArray(local_arr2d7);

	new local_arr2d8[2][8];
	local_arr2d8[0] = local_arr2d8[1]; // warning
	Use2DArray(local_arr2d8);

	new local_arr2d9[2][8];
	++local_arr2d9[0][0];
	Use2DArray(local_arr2d9);

	new local_arr2d10[2][8];
	local_arr2d10[0][0]++;
	Use2DArray(local_arr2d10);
	UseVariable(global_var1);

	global_var2 = 0;
	UseVariable(global_var2);

	UseVariable(global_var3); // warning

	++global_var4;
	UseVariable(global_var4);

	global_var5++;
	UseVariable(global_var5);

	Use1DArray(global_arr1d1);

	global_arr1d2 = "oh, hi!";
	Use1DArray(global_arr1d2);

	global_arr1d3[0] = 1;
	Use1DArray(global_arr1d3);

	Use1DArray(global_arr1d4); // warning

	++global_arr1d5[0];
	Use1DArray(global_arr1d5);

	global_arr1d6[0]++;
	Use1DArray(global_arr1d6);

	Use2DArray(global_arr2d1);

	global_arr2d2[0] = "oh, hi!";
	Use2DArray(global_arr2d2);

	global_arr2d3[0][0] = 1;
	Use2DArray(global_arr2d3);

	Use2DArray(global_arr2d4); // warning

	++global_arr2d5[0][0];
	Use2DArray(global_arr2d5);

	global_arr2d6[0][0]++;
	Use2DArray(global_arr2d6);
}

main()
{
	static arr1d[2] = { 0, 0 };
	static arr2d[2][2] = { { 0, 0 }, { 0, 0 } };
	TestFunc(0, arr1d, arr2d);
}

