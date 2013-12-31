#if debug < 1
	#error Must be compiled with runtime checks
#endif

native strcmp(const string1[], const string2[],
              bool:ignorecase=false, length=cellmax);

#define N 1
#define F 2.34

assert_equal(const s1[], const s2[]) {
	assert(strcmp(s1, s2) == 0);
}

main() {
	assert_equal("foo""bar", "foobar");
	assert_equal("foo" "bar", "foobar");
	assert_equal("foo"	"bar", "foobar");
	assert_equal("foo"#"bar", "foobar");
	assert_equal("foo"##"bar", "foobar");
	assert_equal("foo"#######"bar", "foobar");
	assert_equal(#N, "1");
	assert_equal(#F, "2.34");
	assert_equal(#N #F, "12.34");
}
