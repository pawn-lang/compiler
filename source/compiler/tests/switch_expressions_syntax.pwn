#pragma semicolon 1

Func() return 0;

test_NonConstCaseValue(value)
{
	return switch (value;
		Func(): 0; // error 008: must be a constant expression; assumed zero
		_: 1
	);
}

test_CaseAfterDefault(value)
{
	return switch (value;
		_: 0;
		1: 1  // error 015: "default" case must be the last case in switch statement
	);
}

test_MultipleDefaults(value)
{
	return switch (value;
		_: 0;
		_: 1  // error 016: multiple defaults in "switch"
	);
}

test_MissingSemicolon(value)
{
	return switch (value
		1: 0; // error 001: expected token: ";", but found "-integer value-"
		2: 1
		_: 2  // error 001: expected token: ";", but found "-label-"
	);
}

test_MissingColon(value)
{
	return switch (value;
		1  0; // error 001: expected token: ":", but found "-integer value-"
		2: 1;
		_  2  // error 029: invalid expression, assumed zero
	);
}

test_DuplicateCases(value)
{
	return switch (value;
		1: 0;
		1: 1; // error 040: duplicate "case" label (value 1)
		_: 2
	);
}

test_InvalidRange(value)
{
	return switch (value;
		1..1: 0; // error 050: invalid range
		_:    1
	);
}

test_OverlayingRanges(value)
{
	return switch (value;
		1..10: 0;
		0..9:  1; // error 040: duplicate "case" label (value 1)
		3..8:  2; // error 040: duplicate "case" label (value 3)
		_:     3
	);
}

main()
{
	test_NonConstCaseValue(0);
	test_CaseAfterDefault(0);
	test_MultipleDefaults(0);
	test_MissingSemicolon(0);
	test_MissingColon(0);
	test_DuplicateCases(0);
	test_InvalidRange(0);
	test_OverlayingRanges(0);
}