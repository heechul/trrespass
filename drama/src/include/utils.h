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


extern uint64_t counter;


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
	asm volatile ("dmb sy");
	/* https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb */
#else
	asm volatile ("mfence" : : : "memory");
#endif
}


static inline __attribute__((always_inline)) void lfence() 
{
#if defined(__aarch64__)
	asm volatile ("dsb sy");
#else
	asm volatile ("lfence" : : : "memory");
#endif
}

#if defined(__aarch64__)
#define TIMER_RESOLUTION_AMPLIFIER 25 /* 1 unit = 25 ns (?) */
#else
#define TIMER_RESOLUTION_AMPLIFIER 1
#endif

static inline __attribute__((always_inline)) uint64_t rdtscp(void)
{
#if defined(__aarch64__)
        uint64_t val;
	asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	return val * TIMER_RESOLUTION_AMPLIFIER;
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
        uint64_t val;
	asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	return val * TIMER_RESOLUTION_AMPLIFIER;
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

