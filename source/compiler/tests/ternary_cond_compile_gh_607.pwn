// The ternary operator used to rely on the source code being identical on
// both compilation passes (it collected the heap usage of both branches on
// the first pass and used the values on the second pass). "#if defined main"
// is false on the first pass and true on the second one, so the number of
// ternary operators differs between the passes, which caused an assertion
// failure (or garbage heap allocation in release builds).
#if defined main
Func()
{
	new a[4];
	new x = 0;
	a = x ? "ab" : "xy";
	return a[0];
}
#else
Func(){}
#endif

main()
{
	Func();
}
