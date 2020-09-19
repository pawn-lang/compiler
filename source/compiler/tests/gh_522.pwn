#pragma option -;+

forward public operator=(Tag:a);
public operator=(Tag:a);
public operator=(Tag:a) return _:a;

forward operator=(Tag2:a);
operator=(Tag2:a);
operator=(Tag2:a) return _:a;

forward Func();
public Func();
forward public Func();
public Func(){}
