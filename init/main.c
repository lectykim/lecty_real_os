#include <asm/io.h>
#include <linux/sched.h>
#include <etc/simple_print.h>


#define EXT_MEM_K (*(unsigned short*)0x90002)
#define DRIVE_INFO(*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV(*(unsigned short *)0x901FC)

static long memory_end=0;
static long buffer_memory_end=0;
static long main_memory_start=0;

struct drive_info {char dummy[32];} drive_info;
extern void mem_init(long start,long end);
void main(void)
{

    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    //메모리 설정
    memory_end = (1<<20) + (EXT_MEM_K<<10);
    memory_end &= 0xfffff000;
	if (memory_end > 16*1024*1024)
		memory_end = 16*1024*1024;
	if (memory_end > 12*1024*1024) 
		buffer_memory_end = 4*1024*1024;
	else if (memory_end > 6*1024*1024)
		buffer_memory_end = 2*1024*1024;
	else
		buffer_memory_end = 1*1024*1024;
    main_memory_start = buffer_memory_end;
    mem_init(main_memory_start,memory_end);
    trap_init();
    //인터럽트 내보기, division-by-zero, overflow
    char overflow = 244+244;
    char division_by_zero = 10/0;
    while(1){
            
    }
}