/* vi: set sw=4 ts=4: */
/* common.c
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "common.h"
#include <time.h>

const uint8_t MAC_BCAST_ADDR[6] ALIGN2 = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};



/*****************************************************************
* NAME: monotonic_sec
* ---------------------------------------------------------------
* FUNCTION:  Use /proc/uptime to replace time(0)
* INPUT:    
* OUTPUT:   
* Author:   2011-04-22 Norkay
* Modify:   
****************************************************************/
unsigned long monotonic_sec(void)
{
	unsigned long jiff = 0;
	int fd = open("/proc/uptime", O_RDONLY);
	if(fd>=0)
	{
		int n;
		char buf[128];
		unsigned long u1;
		if((n = read(fd, buf, sizeof buf - 1)) > 0)
		{
			char *stop;
			buf[n] = '\0';
			u1 = strtoul(buf, &stop, 0);
			jiff = u1;
		}
		close(fd);
	}
	return jiff;
}

