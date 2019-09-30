#pragma option -;+

public TestFunc();
public TestFunc() <auto1:STATE1> {}

forward TestFunc(); // no error
public TestFunc(); // no error
forward TestFunc() <auto1:STATE1>; // warning 231: state specification on forward declaration is ignored
public TestFunc() <auto1:STATE1>; // warning 231: state specification on forward declaration is ignored
