#include <float>

// Tag `Float:` is already defined at this moment, so `tagof(Float:)` should
// evaluate to the same value on both the 1'st and the 2'nd compiler passes.
Func1(a = tagof(Float:))
	return a;

// `tagof(Tag:)` returns 0 on the 1'st pass since `Tag:` is not defined yet,
// but it gets defined later (see function `main()`), so on the 2'nd pass
// `tagof(Tag:)` evaluates to a nonzero value, thus causing "error 025: function
// heading differs from prototype". This is solved by ignoring the different
// default values and overriding the old default value obtained at the previous
// function definition with the new one from the current definition when `tagof`
// is used on a tag.
Func2(a = tagof(Tag:))
	return a;

Func3(Tag:a, tag = sizeof(Tag:)) // error 001: expected token: "-identifier-", but found "-label-"
	return (a + Tag:0, tag);

main()
{
	new Tag:t = Tag:0;
	Func1();
	Func2();
	Func3(t);
}
