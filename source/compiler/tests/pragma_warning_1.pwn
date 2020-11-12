forward Func(const &arg); // warning 238: meaningless combination of class specifiers (const reference)

#pragma warning disable 238
#pragma warning push
forward Func(const &arg); // shouldn't warn on this line

#pragma warning enable 238
forward Func(const &arg); // warning 238: meaningless combination of class specifiers (const reference)

#pragma warning pop
forward Func(const &arg); // shouldn't warn on this line

main(){}
