main()
{
    // One line.
	new str1[64] = "hello";
    // Multi line.
	new str2[64] = "
hello
world
";
    // Has comments
	new str3[64] = "/**/";
	new str4[64] = "/*


*/";
	new str5[64] = "
//
";
    // Check commands aren't run mid-string.
	new str6[64] = "
#undef DOESNT_EXIST
";
    // Check replacements are skipped.
	#define A(%0) "%0"
	new str8[3] = A(long_thing);
    // Check line concatenation
	new str9[4] = "01\
2";
    // Raw literals.
	new str10[64] = \"hello";
	new str11[64] = \"
hello
";
    // Packed strings.
	new str12[64] = !"world";
	new str13[64] = !"
world

";
    // Raw packed strings.
	new str14[64] = !\"one";
	new str15[64] = !\"two
";
	new str16[64] = \!"three";
	new str17[64] = \!"


four";
}

