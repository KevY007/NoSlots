/*
	Updated to 0.3.DL by KevY
*/

#include "main.h"

int gen_gpci(char buf[64], unsigned long factor) /* by bartekdvd */
{
	unsigned char out[6*4] = {0};

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 6*4; ++i)
		out[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	out[6*4-1] = 0;

	BIG_NUM_MUL((unsigned long*)out, (unsigned long*)out, factor);

	unsigned int notzero = 0;
	buf[0] = '0'; buf[1] = '\0';

	if (factor == 0) return 1;

	int pos = 0;
	for (int i = 0; i < 24; i++)
	{
		unsigned char tmp = out[i] >> 4;
		unsigned char tmp2 = out[i]&0x0F;
		
		if (notzero || tmp)
		{
			buf[pos++] = (char)((tmp > 9)?(tmp + 55):(tmp + 48));
			if (!notzero) notzero = 1;
		}

		if (notzero || tmp2)
		{
			buf[pos++] = (char)((tmp2 > 9)?(tmp2 + 55):(tmp2 + 48));
			if (!notzero) notzero = 1;
		}
	}
	buf[pos] = 0;

	return pos;
}