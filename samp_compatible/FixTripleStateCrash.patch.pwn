// Test case for TripleStateCrash.patch

#include <a_samp>

main() {
	state ok:no;
}

stock test() <ok:yes, ok:no, ok:dunno> {
	return 1;
}
