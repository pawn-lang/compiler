f1(const &v) {
    #pragma unused v
}
f2(const ...) { }
f3(const v) {
	#pragma unused v
}
f4(...) { }
f5(v) {
	#pragma unused v
}
f6(&v) {
    #pragma unused v
}

main() {
	new a;
	f1(a);
	f2(a);
	f3(a);
	f4(a);
	f5(a);
	f6(a);
}
