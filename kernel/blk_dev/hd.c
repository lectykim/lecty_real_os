#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>

#define MAJOR_NR 3
#include "blk.h"

extern void hd_interrupt(void);

void hd_init(void)
{
    blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
    set_intr_gate(0x2E,&hd_interrupt);
    /*
    x86 PC에서는 하드디스크 컨트롤러가 IRQ 14선을 사용한다.
    IRQ 14는 0x2E에 대응한다. 그래서 hd_interrupt를 0x2#에 등록.

    IRQ2는 슬레이브 PIC가 연결되는 cascade 선이다. hd IRQ14는 슬레이브 쪽 IRQ에 속하므로,
    슬레이브 PIC가 마스터에 신호를 보내려면 IRQ2를 언마스크 해야 됨.
    따라서 &0xfb (011111011)으로 비트 2를 0으로 클리어하여 마스크 2를 언마스크.
    슬레이브 PIC의 IOMR은 I/O Port 0xA1에 위치한다.
    그래서 inb_p(0xA1)로 슬레이브 PIC의 IMR 값을 읽고, &0xbf(01011111)를 통해 비트 6을 0으로 만든 뒤,
    outb(0xA1)로 다시 써서 IRQ14 인터럽트를 허용한다.
    이로써 하드디스크 컨트롤러가 발생시키는 IRQ14 신호가 슬레이브 PIC를 거쳐
    마스터 PIC의 IRQ2로 전파된 후 CPU로 도달할 수 있게 된다.
    */
    outb_p(inb_p(0x21)&0xfb,0x21); 
    outb(inb_p(0xA1)&0xbf,0xA1);
}