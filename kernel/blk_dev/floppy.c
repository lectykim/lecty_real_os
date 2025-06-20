#include <asm/io.h>
#include <asm/system.h>
#include "blk.h"

#define MAJOR_NR 2

extern void floppy_interrupt(void);

void floppy_init(void)
{
    blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
    set_trap_gate(0x26,&floppy_interrupt); 
    /*
    IRQ 0~7까지 마스터 PIC가 관리하고,
    IRQ 번호를 IDT 벡터 번호로 매핑할 때,
    보통 0x20(32부터 시작하므로,) IRQ6은 벡터 0x20 + 6 = 0x26에 대응한다.
    */
    outb(inb_p(0x21)&0x40,0x21);
    /*
    
    8259A PIC의 마스크 레지스터는(IMR, Intterupt Mask Register) I/O 포트 0x21(마스터 PIC)와 0xA1(슬레이브 PIC)에 위치한다
    inb_p(0x21)은 읽어온 IMR 값에서 비트 6을 clear 하는 효과를 내고,
    0x21에 다시 쓴다.
    결국 IRQ6가 unmask되어, 플로피 컨트롤러에서 발생하는 하드웨어 인터럽트가 CPU로 전달될 수 있게 된다.
    */

}