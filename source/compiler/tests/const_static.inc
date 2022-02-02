const static const_val = 2;

UseConstVal()
{
	new a[const_val] = { 0, 1 };
	#pragma unused a
	printf("%d", const_val);
}
