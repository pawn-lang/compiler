#include <console>
#include <string>

main()
{
	new day_seconds = __timestamp % (60 * 60 * 24);
	new hour = day_seconds / (60 * 60);
	new minute = (day_seconds % (60 * 60)) / 60;
	new second = day_seconds % 60;
	new buf[9];
	strformat(buf, _, _, "%02d:%02d:%02d", hour, minute, second);
	printf("result: %d\n", strcmp(buf, __time));
}
