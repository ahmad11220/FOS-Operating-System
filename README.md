# FOS-Operating-System
Operating systems course's project, completing features for FOS: a mini OS.

what is Done ?

Kernel Heap
-allocatin(Next-fit algorithm).
-free.
-find physical address of the given kernel virtual address.
-find kernel virtual address of the given physical one.

Load Environment by env_create()
-create_page_table (create new page table and link it to the directory).

PAGE FAULT HANDLER
-Placement.
-Replacement(Using Modified Clock Algorithm).

CPU Scheduling by MLFQ
-Initialize the MLFQ (sched_init_MLFQ).
-Handle the Scheduler (fos_scheduler).

User Heap
-malloc(user side - kernel side).
-freeMem(user side - kernel side).
