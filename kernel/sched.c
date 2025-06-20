#include <linux/sched.h>
#include <asm/io.h>
#include <asm/system.h>
#define LATCH (1193180/HZ)

#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long)n)<<4)+(FIRST_TSS_ENTRY<<3));
#define _LDT(n) ((((unsigned long)n)<<4)+(FIRST_LDT_ENTRY<<3));
#define ltr(n) __asm__("ltr %%ax"::"a"(_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a"(_LDT(n)))

union task_union{
    struct task_struct task;
    char stack[PAGE_SIZE];
}

static union task_union init_task = {INIT_TASK,};

struct task_struct * task[NR_TASKS] = {&{init_task.task},};

long user_stack [PAGE_SIZE >>2];

struct{
    long *a;
    short b;
} stack_start = {&user_stack[4096>>2],0x10};
void sched_init(void)
{
    int i;
    struct desc_struct *p;
    if(sizeof(struct sigcation) != 16)
        panic("Struct sigcation Must be 16 bytes");
    set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss)); //TSS0 설정하기
    set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt)); //LDT1 설정하기
    p = gdt+2+FIRST_TSS_ENTRY; //GDT의 6번째부터 TSS1, LDT1을 0으로 초기화
    for(i=1;i<NR_TASKS;i++){
        task[i] = NULL;
        p->a = p->b = 0;
        p++;
        p->a = p->b = 0;
        p++;
    }
/*ELAGS에서 NT 비트를 1로 설정하여 문제가 생기지 않도록 함. NT 플래그가 1이면 IRET시 TSS를 사용하지 않기 때문 */
    __asm__("pushfl ; and $0xffffbfff,(%esp) ; popfl");
    ltr(0); //tss를 TR 레지스터에 바인딩
    lldt(0); //ldt를 ldtr레지스터에 바인딩
    outb_p(0x36,0x43); // binary, mode 3, LSB/MSB, ch0 타이머 설정
			/*
			 * 0x43은 PIT(Programable interrupt timer)의 포트를 의미하며,
			 * 0x36은 
			 * bits 7-6 00 -> channel 0 선택
			 * bits 5-4 11 -> Access mode = LSB then MSB (2바이트 순차 전송)
			 * bits 3-1 011 = operating mode = mode 3 (square wave generator)
			 * bit 0 = BCD가 아님. binary 사용
			 * LSB는 16비트 카운터 값의 하위 8비트이며,
			 * MSB는 16비트 카운터 값의 상위 8비트이다.
			 * PIT 카운터 레지스터가 16비트 폭이기 때문에 먼저 하위 바이트를, 그 다음에 상위 바이트를 써 주면 된다.
			 * mode 3은 사각파를 출력한다. timer tick 용도로 쓰기에 적합.
			 * channel 0은 cpu의 시스템 타이머 인터럽트 IRQ0과 연결되어 운영체제의 시간 관리를 담당한다.
			 * 따라서, 해당 코드는 LSB->MSB 순으로 채워 넣어 원하는 주파수(HZ)의 사각파 인터럽트를 channel 0에서 발생시키도록 하는 동작이다.
			 * */
    outb_p(LATCH & 0xff,0x40); //LSB 설정
    outb_p(LATCH >> 8,0x40); //MSB 설정
    outb_p(LATCH >> 8,0x40);
    set_intr_gate(0x20,&timer_interrupt);

    outb(inb_p(0x21)&~0x01,0x21);
    set_system_gate(0x80,&system_call);
}
