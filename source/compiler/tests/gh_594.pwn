main()
{
	new var = 0;
	static const arr[1][1];
	switch (var)
	{
		// Case 1: Array symbol is undefined.
		// NOTE: The "invalid subscript" error doesn't get printed (although the
		// compiler attempts to print it) because the "undefined symbol" error is
		// printed first and error printing is limited to 1 error per statement.
		case 1: return undefined[0]; // error 017: undefined symbol "undefined"
		
		// Case 2: Symbol is not an array.
		case 2: return var[0]; // error 028: invalid subscript (not an array or too many subscripts): "var"

		// Case 3: Curly brackets can only be used on the lowest dimension.
		case 3: return arr{0}[0]; // error 051: invalid subscript, use "[ ]" operators on major dimensions
	}
	return 0;
}
