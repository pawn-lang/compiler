// TODO: Check that string literals are concatenated correctly

native print(const s[]);

#define d1\
    print("ok")
#define d2 \
    print("ok")

#warning this is\
    warning 1
#warning this is\
warning 2
#warning this is \
warning 3
#warning this is \
    warning 4

// single-line comments can span \
multiple lines if you really want it

#pragma deprecated don't\
    use \
this \
    function\
please
f() {}

main() {
    d1;
    d2;
    f();
}
