#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

extern void sha256d_ms(uint32_t *hash, uint32_t *W, const uint32_t *midstate, const uint32_t *prehash);
extern void sha256d_preextend(uint32_t *W);
extern void sha256_init(uint32_t *state);
extern void sha256_transform(uint32_t *state, const uint32_t *block, int swap);
extern void sha256d_prehash(uint32_t *S, const uint32_t *W);

void main()
{
	int i = 0;
	char *p;
	uint32_t hash[8];
	uint32_t prehash[8];

	//uint32_t pdata[32];
	uint32_t data[64];
	uint32_t midstate[8];

        uint32_t pdata[32] = {
                0x428a2f98, 0x71374491, 
                0x3956c25b, 0x59f111f1, 
                0xd807aa98, 0x12835b01, 
                0x72be5d74, 0x80deb1fe, 
                0xe49b69c1, 0xefbe4786, 
                0x2de92c6f, 0x4a7484aa, 
                0x983e5152, 0xa831c66d, 
                0xc6e00bf3, 0xd5a79147, 
                0x27b70a85, 0x2e1b2138, 
                0x650a7354, 0x766a0abb, 
                0xa2bfe8a1, 0xa81a664b, 
                0xd192e819, 0xd6990624, 
                0x19a4c116, 0x1e376c08, 
                0x391c0cb3, 0x4ed8aa4a, 
                0x748f82ee, 0x78a5636f, 
                0x90befffa, 0xa4506ceb, 
        };

	//srand(time(0));
	p = pdata;
	for (i; i < 128; i++)
	{
	//	*p++ = rand() % 256;
	}

        //for (i = 0; i < 32; i++)
        //        printf(" 0x%08x\t", pdata[i]);

	memcpy(data, pdata + 16, 64);
	sha256d_preextend(data); // fill 0 - 128 of data 

	sha256_init(midstate); // fixed 64
	sha256_transform(midstate, pdata, 0); // midstate has been update 8 words, front 16 words of pdata used
	memcpy(prehash, midstate, 32);
	sha256d_prehash(prehash, pdata + 16);  // last 16 words of pdata used with calculated 8 words generate from front 16 words of pdata.


	sha256d_prehash(prehash, pdata + 16);
        //for (i = 0; i < 32; i++)
        //        printf(" 0x%08x\t", pdata[i]);

	sha256d_ms(hash, data, midstate, prehash);


}
