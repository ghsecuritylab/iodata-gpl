#ifndef MYGETTIMEOFDAY_H
#define MYGETTIMEOFDAY_H

#ifdef HAVE_CLOCK_GETTIME
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static char myTimeBuf[80];

int mygettimeofday(struct timeval *tv, struct timezone *tz)
{
#if 1
    struct timeval tv2;
    FILE *fp;
    tv2.tv_sec=0;
    tv2.tv_usec=0;
    

    fp = fopen("/proc/uptime", "r");
    if(fp == NULL) return 0;
    if (fgets(myTimeBuf, sizeof(myTimeBuf), fp) != NULL)
    {
        sscanf(myTimeBuf,"%ld.%ld",&(tv2.tv_sec),&(tv2.tv_usec));
    }
    fclose(fp);
    //return tv.tv_sec * 1000000ULL + tv2.tv_usec;
    
    tv->tv_sec = tv2.tv_sec;
    tv->tv_usec= tv2.tv_usec;
  
    
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tv->tv_sec =ts.tv_sec;
    tv->tv_usec=ts.tv_nsec/1000;
#endif

    return 1;
}

 
time_t mytime(void)
{
	struct timeval tv;

	mygettimeofday(&tv, NULL);

	return tv.tv_sec;
}
#endif

#endif /*MYGETTIMEOFDAY_H*/


