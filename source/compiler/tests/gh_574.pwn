enum { test, test2 };

main()
{
	default:{}
	case test, test2:
	{
		return Func();
	}
}

Func() return 1;
