#pragma option -O0
#include <console>

new arr[2] = { 0, 0 };

main()
{
	printf("%d", arr[1]);
	printf("%d", ++arr[1]);
	printf("%d", arr[1]--);
}
