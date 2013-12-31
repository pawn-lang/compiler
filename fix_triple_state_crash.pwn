main() {
	state ok:no;
}

stock test() <ok:yes, ok:no, ok:dunno> {
	return 1;
}
