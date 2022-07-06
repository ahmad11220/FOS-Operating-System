
#include <inc/lib.h>

#define total_size2 (USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE
#define start_of_user (USER_HEAP_START)/PAGE_SIZE
#define end_of_user (USER_HEAP_MAX)/PAGE_SIZE
void *Next_algo_user(uint32 pages);

//Struct to save alloaction info in user heap
struct info_user
{
	uint32* add;
	int num_pages;
};
struct info_user U_heap[total_size2];

//number of allocations in user heap
int ind_alloc_userheap=0;

uint32 user_pg_status[end_of_user];
uint32 start_heap_va = USER_HEAP_START;
int first_time = 1;

void* malloc(uint32 size)
{
	//set all pages are free-->=0
	if (first_time)
	{

		uint32 Start = USER_HEAP_START;
		if (first_time)
		{
			for(uint32 i =start_of_user ; i<end_of_user ; i++)
			{
				user_pg_status[i]= 0;
			}
			//cprintf("booooooom");
			first_time = 0;
		}
		first_time = 0;
	}

	size = ROUNDUP(size, PAGE_SIZE);
	int pages=(size/PAGE_SIZE);

	//address to start allocate from it
	uint32 Next_alloc;
	Next_alloc = (uint32)Next_algo_user(pages);

	if (Next_alloc==0)
		return NULL ;
	uint32 tmpstart = Next_alloc;

	//change status of page to be not free
	for(int i = 0 ; i<pages ; i++)
	{
		user_pg_status[tmpstart/ PAGE_SIZE] = 1;
		tmpstart += PAGE_SIZE;
	}

	//change to kernel side
	sys_allocateMem((uint32)Next_alloc,size);

	start_heap_va = tmpstart;
	U_heap[ind_alloc_userheap].num_pages=pages;
	U_heap[ind_alloc_userheap].add=(uint32 *)Next_alloc;
	ind_alloc_userheap++;

	return (void*)Next_alloc;
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("smalloc() is not required ..!!");
	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("sget() is not required ..!!");
	return 0;
}


void free(void* virtual_address)
{
	int num_pgs=0;
	for(int k=0;k<=ind_alloc_userheap;k++)
	{
		if(virtual_address==U_heap[k].add)
			num_pgs=U_heap[k].num_pages;
			// cprintf("num is: %d\n",num);
	}

	for(int j=0 ; j< num_pgs ; j++)
	{
		//change status of page to be free
		uint32 Virtual_add = ((int32)virtual_address+(j*PAGE_SIZE)) ;
		user_pg_status[Virtual_add/ PAGE_SIZE] = 0;

	}
	uint32 size =num_pgs*PAGE_SIZE;

	//change to kernel side
	sys_freeMem((uint32)virtual_address,size);
}


void sfree(void* virtual_address)
{
	panic("sfree() is not requried ..!!");
}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}

void *Next_algo_user(uint32 pages)
{
	//counter of free pages
	int free_space = 0;

	uint32 st_alloc = start_heap_va;
	uint32 Max = start_heap_va;

	//start from last allocation
	for(uint32 i=st_alloc ; i<USER_HEAP_MAX ; i+=PAGE_SIZE)
	{
		if (user_pg_status[i/ PAGE_SIZE]==0)
		{
			free_space ++;
		}
		else
		{
			free_space = 0;
		}
		if (free_space == pages)
		{
			st_alloc = i+PAGE_SIZE;
			//cprintf("start is (S) : %x \n",st_alloc);
			//cprintf("start is (i) : %x \n",i+PAGE_SIZE);
			st_alloc -= (pages*PAGE_SIZE);
			break;
		}
	}

	//reached max of kheap
	if (free_space != pages)
	{
		st_alloc = USER_HEAP_START;
		free_space = 0;
		for(uint32 i = st_alloc ; i<Max ; i+=PAGE_SIZE)
		{
			if (user_pg_status[i/ PAGE_SIZE]==0)
				free_space ++ ;
			else
				free_space = 0;
			if (free_space == pages)
			{
				st_alloc = i+PAGE_SIZE;
				st_alloc -= (pages*PAGE_SIZE);
				break;
			}
		}
	}
	if (free_space != pages)
		return 0;

	return (void*)st_alloc;
}

