#pragma option -O0
#include <console>

StringOrigin()
{
	static const string[] = "Hello world";
	return string;
}

ReturnString()
{
	__emit nop; // mark the start of checked P-code
	return StringOrigin();
}

main()
{
	print(ReturnString());
}
