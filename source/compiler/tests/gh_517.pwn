stock Tag:operator=(oper)
	return Tag:oper;

stock operator+(Tag:oper1, Tag:oper2)
	return (_:oper1 + _:oper2);

stock operator+(Tag:oper1, oper2)
	return (_:oper1 - oper2);

main()
{
	new Tag:a;
	a = a + Tag:1;
	a = a + 2;
	a += Tag:3;
	a += 4;
	a = (a += Tag:5);
	a = (a += 6);
	#pragma unused a
}
