main()
{
	enum e1
	{
		e1Elem1[3],
		e1Elem2,
		e1Elem3
	};
	new arr1[e1];
	#pragma unused arr1
	#assert sizeof arr1[e1Elem1] == 3
	#assert sizeof arr1[e1Elem2] == 1
	#assert sizeof arr1[e1Elem3] == 1
	#assert _:e1Elem1 == 0
	#assert _:e1Elem2 == 3
	#assert _:e1Elem3 == 4

	enum e2 (+= 2)
	{
		e2Elem1[3],
		e2Elem2,
		e2Elem3
	};
	new arr2[e2];
	#pragma unused arr2
	#assert sizeof arr2[e2Elem1] == 3
	#assert sizeof arr2[e2Elem2] == 2
	#assert sizeof arr2[e2Elem3] == 2
	#assert _:e2Elem1 == 0
	#assert _:e2Elem2 == 3
	#assert _:e2Elem3 == 5

	enum e3 (*= 2)
	{
		e3Elem1 = 1,
		e3Elem2[3],
		e3Elem3
	};
	new arr3[e3];
	#pragma unused arr3
	#assert sizeof arr3[e3Elem1] == 1
	#assert sizeof arr3[e3Elem2] == 3
	#assert sizeof arr3[e3Elem3] == 1
	#assert _:e3Elem1 == 1
	#assert _:e3Elem2 == 2  // 1 * (1 * 2); the first "1" is the value of the previous
	                        // enum element, the second "1" is the size of the previous
	                        // element, and "2" is the increment (multiplier)
	#assert _:e3Elem3 == 12 // 2 * (3 * 2)

	enum e4 (<<= 2)
	{
		e4Elem1 = 1,
		e4Elem2[3],
		e4Elem3
	};
	new arr4[e4];
	#pragma unused arr4
	#assert sizeof arr4[e4Elem1] == 1
	#assert sizeof arr4[e4Elem2] == 3
	#assert sizeof arr4[e4Elem3] == 1
	#assert _:e4Elem1 == 1
	#assert _:e4Elem2 == 4  // 1 * (1 << 2) = 4
	#assert _:e4Elem3 == 48 // 4 * (3 << 2) = 48

	enum (<<= 0)
	{
		e5Elem1,
		e5Elem2,
		e5Elem3
	};
	#assert _:e5Elem1 == 0
	#assert _:e5Elem2 == 0
	#assert _:e5Elem3 == 0

	enum (*= 1)
	{
		e6Elem1 = 1,
		e6Elem2,
		e6Elem3
	};
	#assert _:e6Elem1 == 1
	#assert _:e6Elem2 == 1
	#assert _:e6Elem3 == 1
}
