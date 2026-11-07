#pragma warning disable 213

enum E_TEST {
    E_TEST_FIELD
}

new E_TEST:values[E_TEST] = {5, ...};

main()
{
    return _:values[0];
}
