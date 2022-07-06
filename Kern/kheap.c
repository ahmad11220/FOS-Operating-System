#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

/////////////////
void *Next_algo_kheap(uint32 pages);
#define total_size (KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE //total number of pages in kernel heap
#define start_of_kern (KERNEL_HEAP_START)/PAGE_SIZE
#define end_of_kern (KERNEL_HEAP_MAX)/PAGE_SIZE

//Struct to save alloaction info in Kernel
struct info_kernel
{
	uint32* add;
	int num_pages;
};
struct info_kernel K_h[total_size];

//number of allocations in kheap
int ind_alloc_kh=0;

//array to know status of each page in Kheap
uint32 KH_pg_status[end_of_kern];

//First allocation to start
uint32 start_heap_va = KERNEL_HEAP_START;

//boolean to start intialization of arr
int first_time = 1;

void* kmalloc(unsigned int size)
{
	//cprintf("starrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrt\n");
	//cprintf("yaaw_st is: %d\n",yaaw_st);
	//cprintf("yaaw_end is: %d\n",yaaw_end);
	//cprintf("total is: %d\n",total_size);

	//set all pages are free-->=0
	if (first_time)
	{
		for(uint32 i =start_of_kern ; i<end_of_kern ; i++)
		{
			KH_pg_status[i]= 0;
		}
		first_time = 0;
	}

	size = ROUNDUP(size, PAGE_SIZE);
	int pages=(size/PAGE_SIZE);

	//address to start allocate from it
	uint32 Next_alloc;
	Next_alloc = (uint32)Next_algo_kheap(pages);
	if (Next_alloc==0)
	return NULL ;
	uint32 tmpstart = Next_alloc;

	//start allocation
	for(uint32 i = 0 ; i<pages ; i++)
	{
		//Allocation

		struct Frame_Info *ptr_frame_info;
		int ret_func=allocate_frame(&ptr_frame_info);
		if(ret_func==E_NO_MEM)
		{
			return NULL;
		}
		ret_func=map_frame(ptr_page_directory, ptr_frame_info, (void*)tmpstart, PERM_PRESENT | PERM_WRITEABLE);
		if(ret_func==E_NO_MEM)
		{
			free_frame(ptr_frame_info);
			return NULL;
		}
		else
		{
			KH_pg_status[tmpstart/PAGE_SIZE]=1;
			tmpstart += PAGE_SIZE;

		}

	}

	start_heap_va = tmpstart;
	K_h[ind_alloc_kh].num_pages=pages;
	K_h[ind_alloc_kh].add=(uint32 *)Next_alloc;
	ind_alloc_kh++;
	return (void*)Next_alloc;

}

void kfree(void* virtual_address)
{
	uint32 *ptr_page_table = NULL;

	int num_pgs=0;

	//searching num of pages to free
	for(int k=0;k<=ind_alloc_kh;k++)
	{
		if(virtual_address==K_h[k].add)
		{
			num_pgs=K_h[k].num_pages;
		}
	}

	//start unmapping
	for(int j=0 ; j< num_pgs ; j++)
	{
		uint32 Virtual_add = ((int32)virtual_address+(j*PAGE_SIZE)) ;
		unmap_frame(ptr_page_directory, (void*) Virtual_add);
		get_page_table(ptr_page_directory,(void*)Virtual_add,&ptr_page_table);
		KH_pg_status[Virtual_add/PAGE_SIZE]=0;
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//num of pages that allocated
	int pages = (start_heap_va-KERNEL_HEAP_START)/PAGE_SIZE;
	uint32 start = KERNEL_HEAP_START;

	for(int i = 0 ; i< pages ; i++)
	{
		struct Frame_Info* ptr_frame_info = NULL;
		uint32* ptr_page_table_index;

		//to get phy address
		ptr_frame_info = get_frame_info(ptr_page_directory, (void*)start, &ptr_page_table_index);
		if(ptr_frame_info != NULL)
		{
			uint32 Phy_Add = to_physical_address(ptr_frame_info);
			if(Phy_Add == physical_address)
					return start;
		}
		start+=PAGE_SIZE;
	}

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//check the range
	if (virtual_address<KERNEL_HEAP_START || virtual_address>KERNEL_HEAP_MAX)
	{
		return 0 ;
	}

	uint32* Pointer_PAge_Table = NULL;
	get_page_table(ptr_page_directory, (void*)virtual_address, &Pointer_PAge_Table);
	if (Pointer_PAge_Table != NULL)
	{
		uint32 ind_addr = ((Pointer_PAge_Table[PTX(virtual_address)] >> 12) * PAGE_SIZE);
		uint32 off = (uint32)virtual_address << 20 >> 20;
		uint32 res = (off + ind_addr);
		uint32 state = Pointer_PAge_Table[PTX(virtual_address)] & PERM_PRESENT;
		if (state != 0)
		{
			return res;
		}
	}
	return 0;
}
void *Next_algo_kheap(uint32 pages)
{
	//counter of free pages
	int free_space = 0;

	uint32 st_alloc = start_heap_va;
	uint32 Max = start_heap_va;

	//start from last allocation
	for(uint32 i=st_alloc ; i<KERNEL_HEAP_MAX ; i+=PAGE_SIZE)
	{


		if (KH_pg_status[i/ PAGE_SIZE]==0)
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
		st_alloc = KERNEL_HEAP_START;
		free_space = 0;
		for(uint32 i = st_alloc ; i<Max ; i+=PAGE_SIZE)
		{
			if (KH_pg_status[i/ PAGE_SIZE]==0)
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
