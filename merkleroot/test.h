
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


#define COINBASE_LEN 210
#define COINBASE_CNT 12


#define bswap_32(x) ((((x) << 24) & 0xff000000u) | (((x) << 8) & 0x00ff0000u) \
                   | (((x) >> 8) & 0x0000ff00u) | (((x) >> 24) & 0x000000ffu))

void sha256d(unsigned char *hash, const unsigned char *data, int len);

static inline uint32_t swab32(uint32_t v)
{
        return bswap_32(v);
}

static inline uint32_t be32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
            ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

static inline uint32_t le32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
            ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void be32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[3] = x & 0xff;
        p[2] = (x >> 8) & 0xff;
        p[1] = (x >> 16) & 0xff;
        p[0] = (x >> 24) & 0xff;
}

static inline void le32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[0] = x & 0xff;
        p[1] = (x >> 8) & 0xff;
        p[2] = (x >> 16) & 0xff;
        p[3] = (x >> 24) & 0xff;
}

struct stratum_job {
        char *job_id;
        unsigned char prevhash[32];
        size_t coinbase_size;
        unsigned char coinbase[COINBASE_LEN];
        unsigned char *xnonce2;
        int merkle_count;
        unsigned char merkle[COINBASE_CNT][32];
        unsigned char version[4];
        unsigned char nbits[4];
        unsigned char ntime[4];
        bool clean;
        double diff;

        unsigned char merkle_root[64];
        /* block header */
        unsigned int data[128];
};

extern void applog(int prio, const char *fmt, ...);

#endif
