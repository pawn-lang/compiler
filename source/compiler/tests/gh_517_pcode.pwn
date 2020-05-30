stock Tag:operator=(oper)
	return Tag:oper;

stock operator+(Tag:oper1, Tag:oper2)
	return (_:oper1 + _:oper2);

stock operator+(Tag:oper1, oper2)
	return (_:oper1 - oper2);

main()
{
	new Tag:a;
	__emit nop; /* mark the start of checked P-code, to avoid any possible collisions */
	a = a + Tag:1;
	a = a + 2;
	a += Tag:3;
	a += 4;
	a = (a += Tag:5);
	a = (a += 6);
	__emit nop; /* mark the end of checked P-code */
	#pragma unused a
}
