#pragma option -;+

forward Func() <auto1:st1>; // warning 231: state specification on forward declaration is ignored
public Func() <auto1:st1> {}

public Func() <auto1:st2>; // warning 231: state specification on forward declaration is ignored
public Func() <auto1:st2> {}

main(){}
