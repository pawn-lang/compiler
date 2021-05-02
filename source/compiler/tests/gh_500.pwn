#pragma deprecated - use "Test4"
forward Test1();

#pragma deprecated
forward Test2();

forward Test3();

stock Test4(){}

main()
{
	Test1(); // error 004: function "Test1" is not implemented - use "Test4"
	Test2(); // error 004: function "Test2" is not implemented
	Test3(); // error 004: function "Test3" is not implemented
}
