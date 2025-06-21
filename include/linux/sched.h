#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64
#define HZ 100

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]
#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags are in one word, max 32 files/proc"
#endif

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4
extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern int free_page_tables(unsigned long from, unsigned long size);

extern void sched_init(void);
extern void schedule(void);
extern void trap_init(void);
extern void panic(const char * str);
extern int tty_write(unsigned minor,char * buf,int count);
/*
FPU(부동 소수점 상태)를 저장하기 위한 자료 구조
멀티태스크 운영체제에서 프로세스 A가 실행 중에 FPU를 사용하고 있었다가,
스케줄러가 프로세스 B로 전환하면 프로레스 A의 FPU상태(레지스터 값 등)를 메모리에 저장해 두어야 한다.
이후 다시 A로 복귀할 때 이 저장된 상태를 복원해야 부동소수점 연산이 올바르게 이루어진다.

*/
struct i387_struct{
    long cwd; //Control Word : FPU연산 제어를 위한 레지스터 값 EX: 반올림 모드,예외마스크, 정밀도 제어
    long swd; //Status Word: FPU 상태를 나타내는 레지스터 값 EX: 예외 플래스, 스택 탑포인터 정보
    long twd; //Tag Word : FPU 스택 레지스터(ST0~ST7) 각각의 내용 유효성을 표시하는 태그
    long fip; //FPU Instruction Pointer 마지막 FPU 명령이 실행된 코드 어드레스를 저장하는 부분
    long fcs; //FPU Code Segment FPU의 코드 세그먼트를 저장하는 공간
    long foo; //FPU Operand Offset: 메모리 오퍼랜드를 사용한 FPU 명령 수행 시, 데이터 포인터(offset) 정보를 저장
    long fos; // FPU Operand Selector: foo와 함께 세그먼트 정보를 담아 세그먼트 기반 주소 계산에 사용
    long st_space[20]; //x87 FPU 레지스터 스택의 실제 80바이트를 저장하기 위한 공간.
}

struct tss_struct{
    long back_link; //16 high bits = zero
    long esp0;
    long ss0; //16 high bits = zero
    long esp1;
    long ss1; //16 high bits = zero
    long esp2;
    long ss2; //16 high bits  =zero
    long cr3;
    long eip;
    long eflags;
    long eax,ecx,edx,ebx;
    long esp;
    long ebp;
    long esi;
    long edi;
    long es; //16 high bits  =zero
    long cs; //16 high bits  =zero
    long ss; //16 high bits  =zero
    long ds; //16 high bits  =zero
    long fs; //16 high bits  =zero
    long hs; //16 high bits  =zero
    long ldt; //16 high bits  =zero
    long trace_bitmap; 
    struct i387_struct i387;
}

struct task_struct{
    long state; //-1 = 실행 중이 아님. 0= 실행 가능함 >0 =멈춤
    long counter; // 프로세스의 남은 time slice, 타이머 인터럽트마다 감소되며 0이 되면 스케줄러가 교체 명령을 내림
    long priority; // 스케줄링 우선순위, 주기적으로 counter을 재계산할 때 더 큰 우선순위를 가진 프로세스가 더 많은 실행 시간을 받게 됨
    long signal; // 대기 중인 시그널들의 비트맵, 시그널 발생 시 이 비트맵에 기록되고, 스케줄링 복귀 시점 중에서 마스크 검사 후 사용자 핸들러 호출이나 기본 동작 수행을 위해 사용됨
    struct sigacation sigacation[32]; //스케줄링 시 발생한 시그널에 따라 action을 달리 함
    long blocked; //마스킹된 시그널의 비트맵
    int exit_code; //프로세스가 종료되었을 때 exit code
    unsigned long start_code,end_code,end_data,brk,start_stack; //각각 프로세스의 코드 시작지점 코드 끝지점 데이터 끝지점 스택 시작지점을 의미한다.
    long pid,father,prgp,session,leader; //각각 프로세스 ID, father = Parent PID, prgp = Process Group Id, session = process group을 묶은 것, leader = session의 리더 프로세스
    unsigned short uid,euid,suid; //uid = 프로세스를 실행시킨 실제 사용자의 ID이다. SUID = 파일을 실행할 때 실행 파일의 '소유자'권한으로 실행되게 한다. = 파일 소유자를 root로 설정하면 suid는 0이 됨.
    unsigned short gid,egid,sgid; //euid는 suid가 설정되어 있다면 suid를 따라가고, 아니라면 uid를 따라간다.
    
}

#endif