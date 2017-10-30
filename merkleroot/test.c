#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <stdint.h>  
#define __USE_GNU 
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>
  
#include "test.h"

static int count = 10;

void applog(int prio, const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);

       {
                char *f;
                int len;
                time_t now;
                struct tm tm, *tm_p;

                time(&now);

                tm_p = localtime(&now);
                memcpy(&tm, tm_p, sizeof(tm));

                len = 50 + strlen(fmt) + 2;
                f = calloc(len, 1);
                sprintf(f, "[%d-%02d-%02d %02d:%02d:%02d] %s\n",
                        tm.tm_year + 1900,
                        tm.tm_mon + 1,
                        tm.tm_mday,
                        tm.tm_hour,
                        tm.tm_min,
                        tm.tm_sec,
                        fmt);
                vfprintf(stderr, f, ap);        /* atomic write to stderr */
                fflush(stderr);
        }
        va_end(ap);
}

/**   
 *   è®¡ç®ä¸¤ä¸ªæ¶é´çé´éï¼å¾å°æ¶é´å·®   
 *   @param   struct   timeval*   resule   è¿åè®¡ç®åºæ¥çæ¶é´   
 *   @param   struct   timeval*   x             éè¦è®¡ç®çåä¸ä¸ªæ¶é´   
 *   @param   struct   timeval*   y             éè¦è®¡ç®çåä¸ä¸ªæ¶é´   
 *   return   -1   failure   ,0   success   
 **/   
int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)   
{   
	int   nsec;   

	if   (   x->tv_sec>y->tv_sec   )   
		return   -1;   

	if   (   (x->tv_sec==y->tv_sec)   &&   (x->tv_usec>y->tv_usec)   )   
		return   -1;   

	result->tv_sec   =   (   y->tv_sec-x->tv_sec   );   
	result->tv_usec   =   (   y->tv_usec-x->tv_usec   );   

	if   (result->tv_usec<0)   
	{   
		result->tv_sec--;   
		result->tv_usec+=1000000;   
	}   

	return   0;   
}   


inline static int set_cpu(int i)  
{  
        cpu_set_t mask;  
        CPU_ZERO(&mask);  

        CPU_SET(i,&mask);  

        printf("thread %lu, i = %d\n", pthread_self(), i);  
        if(-1 == pthread_setaffinity_np(pthread_self() ,sizeof(mask),&mask))  
        {  
                fprintf(stderr, "pthread_setaffinity_np erro\n");  
                return -1;  
        }  
        return 0;  
}  

static int stratum_gen_work()
{
    struct stratum_job job;
    int i = 0;

    job.coinbase_size = COINBASE_LEN;
    job.merkle_count = COINBASE_CNT;

//    applog(0, "  %s\n", __func__);
    memset(job.data, 0, 128);
    sha256d(job.merkle_root, job.coinbase, job.coinbase_size);
    for (i = 0; i < job.merkle_count; i++) {
        memcpy(job.merkle_root + 32, job.merkle[i], 32);
        sha256d(job.merkle_root, job.merkle_root, 64);
    }

    job.data[0] = le32dec(job.version);
    for (i = 0; i < 8; i++)
        job.data[1 + i] = le32dec((uint32_t *)job.prevhash + i);
    for (i = 0; i < 8; i++)
        job.data[9 + i] = be32dec((uint32_t *)job.merkle_root + i);

}

void * job_thread(void *info)
{
    int i = 0;

    if(set_cpu(*(int *)info))  
    {  
        return NULL;  
    }  
    applog(0, " start %s, i: %d\n", __func__, *(int *)info);

    for (i = 0; i < count; i++)
        stratum_gen_work();
    
    applog(0, " end %s\n", __func__);
}

int main(int argc, char **argv)
{

    int cpu_nums = sysconf(_SC_NPROCESSORS_CONF);  

    if (argc > 1)
        count = atoi(argv[1]);

    applog(0, " total  start %s\n", __func__);
    printf("cpu_numbs = %d\n", cpu_nums);

    pthread_t job_thread_t[cpu_nums];
    int tmp[cpu_nums];


    struct timeval start;
    struct timeval end;
    struct timeval consume_time;

    gettimeofday(&start, NULL);
    //stratum_gen_work();
    for (int j = 0; j < cpu_nums; j++) {
        tmp[j] = j;
        if(pthread_create(&job_thread_t[j], 0, job_thread, &tmp[j])) applog(0, "Thread creation failed");
        //pthread_join(job_thread_t[j], 0);
    }
    for (int j = 0; j < cpu_nums; j++) {
        //if(pthread_create(&job_thread_t[j], 0, job_thread, &tmp[j])) applog(0, "Thread creation failed");
        pthread_join(job_thread_t[j], 0);
    }
    gettimeofday(&end, NULL);

    timeval_subtract(&consume_time, &start, &end);
    double over_time = (consume_time.tv_usec + consume_time.tv_sec * 1000000);

    applog(0, " total test %d times, end %s, total: %f microseconds, one merkleroot cost %f microseconds\n", count, __func__, over_time, over_time / (double)count);
}
