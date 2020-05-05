const TD_0 = 0;
enum { TD_1 = 1 };

new Tag:tagged_arr[2];
new untagged_arr[2];

enum eArrayStructure
{
	Tag:aTagVal,
	bool:aBoolVal
};
new structured_arr[eArrayStructure];

main()
{
	tagged_arr[TD_0] = Tag:0;
	tagged_arr[TD_1] = Tag:0;

	tagged_arr[TD_0] = 0; // tag mismatch
	tagged_arr[TD_1] = 0; // tag mismatch

	untagged_arr[TD_0] = Tag:0; // tag mismatch
	untagged_arr[TD_1] = Tag:0; // tag mismatch

	untagged_arr[TD_0] = 0;
	untagged_arr[TD_1] = 0;

	structured_arr[aTagVal] = Tag:0;
	structured_arr[aBoolVal] = false;

	structured_arr[aTagVal] = false; // tag mismatch
	structured_arr[aBoolVal] = Tag:0; // tag mismatch

	structured_arr[TD_0] = Tag:0; // tag mismatch
	structured_arr[TD_1] = false; // tag mismatch

	#pragma unused tagged_arr, untagged_arr, structured_arr
}
