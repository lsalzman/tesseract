// implementation of generic tools

#include "cube.h"

#ifndef WIN32
#include <unistd.h>
#endif

int guessnumcpus()
{
    int numcpus = 1;
#ifdef WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    numcpus = (int)info.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
    numcpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return max(numcpus, 1);
}
    
////////////////////////// rnd numbers ////////////////////////////////////////

#define N (624)             
#define M (397)                
#define K (0x9908B0DFU)       

static uint state[N];
static int next = N;

void seedMT(uint seed)
{
    state[0] = seed;
    for(uint i = 1; i < N; i++)
        state[i] = seed = 1812433253U * (seed ^ (seed >> 30)) + i;
    next = 0;
}

uint randomMT()
{
    int cur = next;
    if(++next >= N)
    {
        if(next > N) { seedMT(5489U + time(NULL)); cur = next++; }
        else next = 0;
    }
    uint y = (state[cur] & 0x80000000U) | (state[next] & 0x7FFFFFFFU);
    state[cur] = y = state[cur < N-M ? cur + M : cur + M-N] ^ (y >> 1) ^ (-int(y & 1U) & K);
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    y ^= (y >> 18);
    return y;
}

