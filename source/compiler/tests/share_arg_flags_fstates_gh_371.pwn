f0(str[]) <fstate : a>
{
	new a = str[0];
	#pragma unused a
}

f0(str[]) <fstate : b>
{
	new a = str[0] + str[1];
	#pragma unused a
}

f1(str[]) <fstate : a>
{
	new a = str[0];
	#pragma unused a
}

f1(str[]) <fstate : b>
{
	str[0] = 5;
}

f2(const str[]) <fstate : a>
{
	new a = str[0];
	#pragma unused a
}

f2(const str[]) <fstate : b>
{
	new a = str[0] + str[1];
	#pragma unused a
}

f3(str[]) <fstate : a>
{
	str[0] = 5;
}

f3(str[]) <fstate : b>
{
    new a = str[0];
	#pragma unused a
}

main()
{
	state fstate : a;
	new arr[10];
	f0(arr);
	f1(arr);
	f2(arr);
	f3(arr);
}
