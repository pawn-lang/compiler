#if debug < 1
	#error Must be compiled with runtime checks
#endif

forward force_compile();

native print(const s[]);
native strcmp(const string1[], const string2[],
              bool:ignorecase=false, length=cellmax);

static assert_equal(const s1[], const s2[]) {
	assert(strcmp(s1, s2) == 0);
}

test_concat() {
	assert_equal("foo""bar", "foobar");
	assert_equal("foo" "bar", "foobar");
	assert_equal("foo"	"bar", "foobar");
	assert_equal("foo"#"bar", "foobar");
	assert_equal("foo"##"bar", "foobar");
	assert_equal("foo"#######"bar", "foobar");
}

test_stringize() {
	#define N 1
	#define F 2.34
	assert_equal(#N, "1");
	assert_equal(#F, "2.34");
	assert_equal(#N #F, "12.34");
	#undef N
	#undef F
}

main() {
	test_concat();
	test_stringize();
}

ternary_op() {
	new a = 5;
	print((a == 5) ? "is five" : !"is not five");
	print((a != 5) ? !"is not five" : "is five");
}

public force_compile() {
	ternary_op();
}
