Func2(const arr[3])
{
	return arr[0];
}

Func1(const arr[])
{
	Func2(arr[0]); // Must be "error 047: array sizes do not match, or destination array
	               // is too small", not "error 007: operator cannot be redefined"
	               // (a typo in the error code made by the original Pawn author).
}

main()
{
	static const arr[1];
	Func1(arr);
}
