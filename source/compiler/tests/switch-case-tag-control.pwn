#include <float>

main()
{
	new x = 0;
	switch (x)
	{
		case 0: {} // no tag (ok)
		case true: {} // bool (should work since "bool" is a weak tag)
		case 2.0: {} // Float: (tag mismatch; "Float" is a strong tag)
	}
	new Float:f = 0.0;
	switch (f)
	{
		case 0.0: {} // Float (ok)
		case true: {} // bool (tag mismatch)
		case 2: {} // no tag (tag mismatch)
	}
	const Tag:THREE = Tag:3;
	switch (x)
	{
		case -2 .. -1: {} // no tag (ok)
		case 0 .. true: {} // bool (should work since "bool" is a weak tag)
		case 2 .. THREE: {} // Tag: (tag mismatch; "Tag" is a strong tag)
	}
}
