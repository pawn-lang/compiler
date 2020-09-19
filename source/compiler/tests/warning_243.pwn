main()
{
	const CONST_1 = 1;
	new var = 0;

	switch (0) {} // warning 243
	switch (CONST_1) {} // warning 243
	switch (CONST_1 + 1) {} // warning 243
	switch (var) {}
}
