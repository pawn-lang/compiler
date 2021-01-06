// This file should pass with \r\n and \n.

new str1[] = "hello";
#assert sizeof(str1) == 6

new str2[] = "hello\
";
#assert sizeof(str2) == 6

new str3[] = "hello\
           ";
#assert sizeof(str3) == 6

new str4[] = "hello\
           world";
#assert sizeof(str4) == 11

new str5[] = "hello
           world";
#assert sizeof(str5) == 23

#pragma unused str1, str2, str3, str4, str5

main(){}
