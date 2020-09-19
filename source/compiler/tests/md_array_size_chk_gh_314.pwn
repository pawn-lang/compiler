new arr1[];
new arr2[5][];
new arr3[5][][5];
new arr4[5][5];
new arr5[][];

f1(arr[]) {
	#pragma unused arr
}
f2(arr[5][]) {
	#pragma unused arr
}
f3(arr[5][][5]) {
	#pragma unused arr
}
f4(arr[5][5]) {
	#pragma unused arr
}
f5(arr[][]) {
	#pragma unused arr
}

main () {
	arr1[0] = 0;
	arr2[0][0] = 0;
	arr3[0][0][0] = 0;
	arr4[0][0] = 0;
	arr5[0][0] = 0;

	new a = sizeof(arr1);
	new b = sizeof(arr1[]);
	new c = sizeof(arr5[][]);
	#pragma unused a, b, c

	f1(arr1);
	f2(arr2);
	f3(arr3);
	f4(arr4);
	f5(arr5);
}
