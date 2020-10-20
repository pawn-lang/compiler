#include <console>

enum (<<= 32)
{
	a0 = 0
};

enum (<<= -1)
{
	b0 = 0
};

enum (<<= 1)
{
	c0 = 0
};
enum (<<= 31)
{
	d0 = 0
};

enum (<<= 1)
{
	e0 = 1,
	e1,
	e2,
	e3,
	e4,
	e5,
	e6,
	e7,
	e8,
	e9,
	e10,
	e11,
	e12,
	e13,
	e14,
	e15,
	e16,
	e17,
	e18,
	e19,
	e20,
	e21,
	e22,
	e23,
	e24,
	e25,
	e26,
	e27,
	e28,
	e29,
	e30,        // 0b01000000000000000000000000000000
	e31,        // 0b10000000000000000000000000000000
	e32,        // 0b00000000000000000000000000000000 (overflow)
	e33         // 0b00000000000000000000000000000000
};

enum (<<= 1)
{
	f0 = 1,
	f1,
	f2,
	f3,
	f4,
	f5,
	f6,
	f7,
	f8,
	f9,
	f10,
	f11,
	f12,
	f13,
	f14,
	f15,
	f16,
	f17,
	f18,
	f19,
	f20,
	f21,
	f22,
	f23,
	f24,
	f25,
	f26,
	f27,
	f28,
	f29,
	f30,
	f31,        // 0b10000000000000000000000000000000
	f32 = f31-1,// 0b01111111111111111111111111111111
	f33,        // 0b11111111111111111111111111111110
	f34,        // 0b11111111111111111111111111111100 (overflow)
	f35         // 0b11111111111111111111111111111000 (overflow)
};

enum (<<= 0)
{
	g1 = 1,
	g2,
	g3
};

enum (<<= 32) // warning 241
{
	h1 = 1,
	h2,
	h3
};

main()
{
	new var = 1;

	// Case 1: Both values are compile-time constants
	printf("%d", 1 << -1); // warning 241
	printf("%d", 1 << -2); // warning 241
	printf("%d", 1 >> -1); // warning 241
	printf("%d", 1 >> -2); // warning 241
	printf("%d", 1 >>> -1); // warning 241
	printf("%d", 1 >>> -2); // warning 241
	printf("%d", 1 << 0);
	printf("%d", 1 << 1);
	printf("%d", 1 >> 0);
	printf("%d", 1 >> 1);
	printf("%d", 1 >>> 0);
	printf("%d", 1 >>> 1);
	printf("%d", 1 << 30);
	printf("%d", 1 << 31);
	printf("%d", 1 >> 30);
	printf("%d", 1 >> 31);
	printf("%d", 1 >>> 30);
	printf("%d", 1 >>> 31);
	printf("%d", 1 << 32); // warning 241
	printf("%d", 1 << 33); // warning 241
	printf("%d", 1 >> 32); // warning 241
	printf("%d", 1 >> 33); // warning 241
	printf("%d", 1 >>> 32); // warning 241
	printf("%d", 1 >>> 33); // warning 241

	// Case 2: Only the shift count is constant
	printf("%d", var << -1); // warning 241
	printf("%d", var << -2); // warning 241
	printf("%d", var >> -1); // warning 241
	printf("%d", var >> -2); // warning 241
	printf("%d", var >>> -1); // warning 241
	printf("%d", var >>> -2); // warning 241
	printf("%d", var << 0);
	printf("%d", var << 1);
	printf("%d", var >> 0);
	printf("%d", var >> 1);
	printf("%d", var >>> 0);
	printf("%d", var >>> 1);
	printf("%d", var << 30);
	printf("%d", var << 31);
	printf("%d", var >> 30);
	printf("%d", var >> 31);
	printf("%d", var >>> 30);
	printf("%d", var >>> 31);
	printf("%d", var << 32); // warning 241
	printf("%d", var << 33); // warning 241
	printf("%d", var >> 32); // warning 241
	printf("%d", var >> 33); // warning 241
	printf("%d", var >>> 32); // warning 241
	printf("%d", var >>> 33); // warning 241

	printf("%d", 1 << var); // Just to make sure the warning works only
	                        // if the shift count is a constant value
}
