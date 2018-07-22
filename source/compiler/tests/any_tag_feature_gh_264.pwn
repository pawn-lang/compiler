f1({...}:arg) {
	#pragma unused arg
}

g1(arg) {
	#pragma unused arg
}

g1_1({Float}:arg) {
	#pragma unused arg
}

f2({...}:arg = Tag:5) {
	#pragma unused arg
}

g2(arg = Tag:5) { // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	#pragma unused arg
}

g2_1({Float}:arg = Tag:5) { // warning 213: tag mismatch: expected tag "Float", but found "Tag"
	#pragma unused arg
}

f3({...}:arg = tag:5) {
	#pragma unused arg
}

g3(arg = tag:5) {
	#pragma unused arg
}

g3_1({Float, tag}:arg = tag:5) { // warning 213: tag mismatch: expected tag "Float", but found "Tag"
	#pragma unused arg
}

f4(const {...}:arg[] = {Tag:1}){
	#pragma unused arg
}

g4(const arg[] = {Tag:1}) { // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	#pragma unused arg
}

f5(const {...}:arg[][] = {{Tag:1}}) {
	#pragma unused arg
}

g5(const arg[][] = {{Tag:1}}) { // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	#pragma unused arg
}

f6({...}:&arg = Tag:5) {
	#pragma unused arg
}

g6(&arg = Tag:5) { // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	#pragma unused arg
}

f7({...}:...) {

}

g7(...) {

}


main() {
	f1(Tag:0);
	f1(0);
	f1(tag:0);
	g1(Tag:0); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	g1(0);
	g1(tag:0);
	g1_1(Tag:0);// warning 213: tag mismatch: expected tag "Float", but found "Tag"
	g1_1(Float:0);
	g1_1(0);// warning 213: tag mismatch: expected tag "Float", but found none ("_")
	g1_1(tag:0); // warning 213: tag mismatch: expected tag "Float", but found "tag"

	f2();
	f2(Tag:0);
	g2();
	g2(Tag:0); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	g2_1();
	g2_1(Tag:0); // warning 213: tag mismatch: expected tag "Float", but found "Tag"

	f3();
	f3(Tag:0);
	g3();
	g3(Tag:0); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
	g3_1();
	g3_1(Tag:0); // warning 213: tag mismatch: expected tags "Float", or "tag"; but found "Tag"

	f4();
	f4({Tag:0});
	g4();
	g4({Tag:0});

	new Tag:tmp[][] = {{0}}; // warning 213: tag mismatch: expected tag "Tag", but found none ("_")
	f5();
	f5(tmp);
	g5();
	g5(tmp); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"

	f6();
	f6(tmp[0][0]);
	g6();
	g6(tmp[0][0]); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"

	f7(Tag:5);
	g7(Tag:5); // warning 213: tag mismatch: expected tag none ("_"), but found "Tag"
}

new arr1[5];
new arr2[] = {0};
new arr3[] = Tag:{ 0 };

#pragma unused arr1
#pragma unused arr2
#pragma unused arr3

f15({..., Float}:arg) { // error 001: expected token: "}", but found ","

}

f16({Float, ...}:arg) { // error 001: expected token: "-tagname-", but found "..."

}

#pragma unused f15
#pragma unused f16

enum ITEM {
	Float:item
};
new arr4[ITEM] = {Tag:0};
#pragma unused arr4
