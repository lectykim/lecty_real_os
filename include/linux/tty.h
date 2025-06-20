#ifndef _TTY_H
#define _TTY_H

#include <termio.h>

#define TTY_BUF_SIZE 1024

struct tty_queue{
    unsigned long data;
    unsigned long head;
    unsigned long tail;
    struct task_struct * proc_list;
    char buf[TTY_BUF_SIZE];
}

/* 해당 매크로들은 순환 버퍼의 경계를 넘어갈 때 버퍼 크기만큼 감싸서 0부터 시작하도록 하는 코드이다
해당 코드와 같으 분기(if문) 없이 단일 비트 연산(and)로 순환 처리를 할 수 있어 커널같은 성능 민감 코드에서 효율적이다.
일반적인 모듈러 연산 같은 경우에는 컴파일러/아키텍처에 따라 나눗셈이나 반복뺄셈을 생성할 수 있어 비용이 크다.*/
#define INC(a) ((a) = ((a)+1)&(TTY_BUF_SIZE-1))
#define DEC(a) ((a)= ((a)-1)&(TTY_BUF_SIZE-1))
#define EMPTY(a) ((a).head == (a).tail)
#define LEFT(a) (((a).tail - (a).head-1)&(TTY_BUF_SIZE-1))
#define LAST(a) ((a).buf[(TTY_BUF_SIZE-1)&((a).head-1)])
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a).head-(a).tail)&(TTY_BUF_SIZE-1))
#define GETCH(queue,c) \
(void)({c=(queue).buf[(queue).tail];INC((queue).tail);})
#define PUTCH(queue,c) \
(void)({(queue).buf[(queue).head]=(c);INC((queue).head);})

#define INTR_CHAR(tty) ((tty)->termios.c_cc[VINTR])
#define QUIT_CHAR(tty) ((tty)->termios.c_cc[VQUIT])
#define ERASE_CHAR(tty) ((tty)->termios.c_cc[VERASE])
#define KILL_CHAR(tty) ((tty)->termios.c_cc[VKILL])
#define EOF_CHAR(tty) ((tty)->termios.c_cc[VEOF])
#define START_CHAR(tty) ((tty)->termios.c_cc[VSTART])
#define STOP_CHAR(tty) ((tty)->termios.c_cc[VSTOP])
#define SUSPEND_CHAR(tty) ((tty)->termios.c_cc[VSUSP])

struct tty_struct{
    struct termios termios;
    int pgrp;
    int stopped;
    void(*write)(struct tty_struct * tty);
    struct tty_queue read_q;
    struct tty_queue write_q;
    struct tty_queue secondary;
};

extern struct tty_struct tty_table[];
/*	intr=^C		quit=^|		erase=del	kill=^U
	eof=^D		vtime=\0	vmin=\1		sxtc=\0
	start=^Q	stop=^S		susp=^Z		eol=\0
	reprint=^R	discard=^U	werase=^W	lnext=^V
	eol2=\0
*/


#endif