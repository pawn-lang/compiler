native Tag:operator+(Tag:a, Tag:b); // error 001: expected token: "=", but found ";"

// Make sure that valid native operator and function declarations aren't affected
native Tag2:operator+(Tag2:a, Tag2:b) = NativeFunc;
native Tag2:NativeFunc(Tag2:a, Tag2:b);
native NativeFunc2(a,b) = NativeFunc;

main(){}
