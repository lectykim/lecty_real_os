#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \ //디스크립터의 limit 필드를 104바이트로 설정 <- 104바이트는 tss구조체 크기. 메모리 %1은 디스크립터 바이트. n[0]~n[1]
         "movw %%ax,%2 \n\t" \ //베이스 주소 (addr)의 하위 16비트를 디스크립터 베이스 필드에 저장. %2는 디스크립터 바이트 n[2]~n[3]
         "rorl $16, %%eax \n\t" \ //eax <- (addr>>16) 저장
         "movb %%al,%3 \n\t" \ // addr의 상위 16비트~23비트를 디스크립터 n[4]에 저장
         "movb $"type",%4 \n\t"\ //디스크립터의 타입 dpl 비트를 n[5]에 기록한다, 여기서 매크로 인자로 0x89는 tss이며, 0x82는 ldt를 의미한다.
         "movb $0x00,%5 \n\t"\ //n[6]은 0으로 채운다. 
         "movb %%ah,%6 \n\t"\ //addr>>24 를 n[7]에 저장할 수 있음.
         "rorl $16,%%eax" \ //eax를 원래대로 복원.
         ::"a"(addr),"m"(*(n)),"m" (*(n+2)),"m" (*(n+4)),\
         "m"(*(n+5)),"m",(*(n+6)),"m"(*(n+7))\
)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *)(n)),addr,"0x89") //0x89, P=1, DPL=0, Type=1001b 32bit TSS
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char*)(n)),addr,"0x82") //0x82= P=1, DPL=0, Type=0010b LDT Descriptor 

#define sti() __asm__ ("sti"::)

#define move_to_user_mode()\
__asm__ ("movl %%esp,%%eax \n\t"\
    "pushl $0x17 \n\t" \ //스택에 ss의 값, 0x17(10111)을 넣는다. (권한 레벨 3, LDT, 데이터 세그먼트)
    "pushl %%eax \n\t"\ //esp 값을 스택에 넣음.
    "pushfl \n\t"\ //eflags 삽입
    "pushl $0x0f \n\t" \ //스택에 cs, 0x0f(1111)을 넣는다. (권한 레벨 3,LDT, 코드 세그먼트)
    "pushl $1f \n\t" \ //eip를 삽입한다.
    "iret \n" \ //반환, 이 때 권한 레벨 3에서 권한 0으로 변경된다.
    "1:\t movl $0x17,%%eax\n\t"\ //다음 코드는 ds,es,fs,gs, ss값을 동일하게 설정한다.
    "movw %%ax %%ds\n\t"\
    "movw %%ax,%%es \n\t"\
    "movw %%ax,%%fs\n\t"\
    "movw %%ax,%%gs"\
    :::"ax")