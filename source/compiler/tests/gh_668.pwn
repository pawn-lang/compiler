new global_array[0] = { 0 }; // error 009

main()
{
	new local_array[0] = { 0 }; // error 009
	return global_array[0] + local_array[0];
}
