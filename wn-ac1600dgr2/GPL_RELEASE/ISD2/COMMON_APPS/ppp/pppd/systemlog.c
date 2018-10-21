#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>


/*****************************************************************
* NAME: SystemLog
* ---------------------------------------------------------------
* FUNCTION:  
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
****************************************************************/
static char SystemLogBuf[256];
int SystemLog(char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsnprintf(SystemLogBuf, sizeof(SystemLogBuf), format, ap);
    /* printf("----->%s\n", SystemLogBuf); */
    openlog( "systemd", LOG_NDELAY, LOG_LOCAL1);

    syslog(LOG_INFO, SystemLogBuf);
    closelog();

    return 0;
}
/*
int main()
{
	SystemLog("aaa");
}
*/