#pragma once 

#include <stdint.h>
#include <stdio.h>


#define BIT(x) (1ULL<<(x))
#define KB(x) ((x)<<10ULL)
#define MB(x) ((x)<<20ULL)
#define GB(x) ((x)<<30ULL)
#define CL_SHIFT 6
#define CL_SIZE 64


#define F_CLEAR 	0L
#define F_VERBOSE 	BIT(0)
#define F_EXPORT 	BIT(1)

#define MEM_SHIFT			(30L)
#define MEM_MASK			0b11111ULL << MEM_SHIFT				
#define F_ALLOC_HUGE 		BIT(MEM_SHIFT)
#define F_ALLOC_HUGE_1G 	F_ALLOC_HUGE | BIT(MEM_SHIFT+1)
#define F_ALLOC_HUGE_2M		F_ALLOC_HUGE | BIT(MEM_SHIFT+2)
#define F_POPULATE			BIT(MEM_SHIFT+3)



//----------------------------------------------------------
// 			Static functions

static inline __attribute__((always_inline)) void clflush(volatile void *p)
{
#if defined(__aarch64__)
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (p) : "memory");
#else
	asm volatile("clflush (%0)\n"
		:: "r" (p) : "memory");
#endif
}


static inline __attribute__((always_inline)) void mfence() 
{
#if defined(__aarch64__)
	asm volatile ("dmb sy" : : : "memory");
	/* https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb */
#else
	asm volatile ("mfence" : : : "memory");
#endif
}


static inline __attribute__((always_inline)) void lfence() 
{
#if defined(__aarch64__)
	asm volatile ("isb": : : "memory");
#else
	asm volatile ("lfence" : : : "memory");
#endif
}

#if defined(__aarch64__)
#define USE_FAST_COUNTER   1
#include <pthread.h>
static volatile uint64_t counter = 0;
static pthread_t count_thread;

static void *countthread(void *dummy) {
	uint64_t local_counter = 0;
	while (1) {
#if USE_FAST_COUNTER==1
		local_counter++;
		counter = local_counter;
#else
		counter++;
#endif
	}
	return NULL;
}

static int start_counter()
{
	int rr = pthread_create(&count_thread, 0, countthread , 0);
	if (rr != 0) {
		return -1;
	}
	fprintf(stderr, "[LOG} %s\n", "Waiting the counter thread...");
	while(counter == 0) {
		asm volatile("DSB SY");
	}
	fprintf(stderr, "[LOG] Done: %ld\n", counter);
	return 0;
}
#endif // __aarch64__

static inline __attribute__((always_inline)) uint64_t rdtscp(void)
{
#if defined(__aarch64__)
	if (counter == 0) start_counter();

	lfence();
	return counter;
	// uint64_t val;
	// lfence();
	// asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	// return val;
#else
	uint64_t lo, hi;
	asm volatile("rdtscp\n"
		: "=a" (lo), "=d" (hi)
		:: "%rcx");
	return (hi << 32) | lo;
#endif
}


static inline __attribute__((always_inline)) uint64_t rdtsc(void)
{
#if defined(__aarch64__)
	lfence();
	return counter;
	// uint64_t val;
	// asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	// return val;
#else
	uint64_t lo, hi;
	asm volatile("rdtsc\n"
		: "=a" (lo), "=d" (hi)
		:: "%rcx");
	return (hi << 32) | lo;
#endif
}


//----------------------------------------------------------
// 			Memory alloc


typedef struct {
	char* 		buffer;
	uint64_t 	size;
	uint64_t 	flags; 
} mem_buff_t;


int alloc_buffer(mem_buff_t* mem);

int free_buffer(mem_buff_t* mem);


//----------------------------------------------------------
// 			Helpers 
int gt(const void * a, const void * b);

double mean(uint64_t* vals, size_t size);

uint64_t median(uint64_t* vals, size_t size);

char* bit_string(uint64_t val);

