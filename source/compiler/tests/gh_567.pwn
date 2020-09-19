#pragma option -O0
#include <console>

StringOrigin()
{
	static const string[] = "Hello world";
	return string;
}

ReturnString()
{
	static x = 0;
	__emit nop; // mark the start of checked P-code
	if (x)
		return StringOrigin();
	return StringOrigin(), StringOrigin();
}

main()
{
	print(ReturnString());
}
