#include <asm/io.h>
#include <asm/system.h>

extern void rs1_interrupt(void);
extern void rs2_interrupt(void);

static void init(int port)
{
    outb_p(0x80,port+3); 
    /*DLAB을 1로 설정해서, Line Control Register의 최상위 비트(DLAB)를 1로 올려서,
     다음에 쓰는 port/port+1이 Divisor Latch(속도 설정 레지스터)가 되도록 선택한다. */
    outb_p(0x30,port); 
    /*Divisor Latch Low 바이트에 0x30(=48)을 저장한다. 기준 분주기를 48로 설정한다 -> 115200/48 = 2400bps
    */
    outb_p(0x00,port+1);
    /*Divisor Latch High 바이트에 0을 설정해서, 전체 divisor = 0x0030이 되게 한다. */
    outb_p(0x03,port+3);
    /* DLAB을 다시 0으로 설정해서 DLAB을 끈다. */
    outb_p(0x0b,port+4);
    /* DTR, RTS, OUT_2를 킨다. 
    이것은 IRQ 라우팅 허용 및 데이터 송수신 회선을 준비하는 것이다.
    */
    outb_p(0x0d,port+1);
    /* 모든 인터럽트를 켠다.
    RX 데이터 수신, Tx 준비, 모뎀 상태 변화 인터럽트를 모두 활성화한다.
    */
    (void)inb(port); 
    /* Receive Buffer Register을 읽는다. 잔류 데이터를 비우고 내부 상태를 리셋하는 과정
    일부 칩에서는 초기화를 트리거하는 역할을 한다.
    */
}

void rs_init(void)
{
    set_intr_gate(0x24,rs1_interrupt); //시리얼 포트 1의 인터럽트 설정
    set_intr_gate(0x23,rs2_interrupt); //시리얼 포트 2의 인터럽트 설정
    init(tty_table[1],read_q.data); //시리얼 포트 1의 초기화
    init(tty_table[2],read_q.data); //시리얼 포트 2의 초기화
    outb(inb_p(0x21)&0xE7,0x21); //IRQ3,IRQ4 허용
}