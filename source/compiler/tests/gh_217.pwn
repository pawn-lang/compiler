// TODO: Check that string literals are concatenated correctly

#include <console>

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
//multiple lines if you really want it
// no they can't, only with more //s.

#pragma deprecated don't\
    use \
this \
    function\
please
f() {}

#pragma deprecated don't\
    use \
this \
    function\
please
forward p();

public p() {}

main() {
    d1;
    d2;
    f();
    p();
}
