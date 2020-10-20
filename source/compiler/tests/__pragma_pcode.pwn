NakedFunc()
{
	__emit nop; // this should be the only instruction generated for this function
	__pragma("naked");
}

main()
{
	return NakedFunc();
}
