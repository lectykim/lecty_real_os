!
! setup.s는 BIOS에서 신뢰할 수 있게 시스템 데이터를 가져옵니다.
! 그리고 적절한 시스템 메모리에 넣습니다/
!  setup.s와 시스템은 부트블록에 로드됩니다.
!
! 이 코드는 memory/dsk/other 파라미터를 묻고 있습니다.
! 그리고 안전한 장소인 0x90000-0x901FF에 적재합니다.
! 부트 블록이 사용되는 장소는 보호 모드로 보호됩니다.

! 이 변수는 bootsect.s랑 같아야 좋아요.
INITSEG = 0x9000
SYSSEG = 0x1000
SETUPSEG = 0x9020
.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text
entry start
start:

! 현재 커서 저장하기
    mov ax,#INITSEG
    mov ds,ax
    mov ah,#0x03 !read cursor pos
    xor bh,bh
    int 0x10 ! 잘 알려진 곳에 저장
    mov [0], dx ! it from 0x90000

! 메모리 크기 알아오기 (확장 메모리, kB)
    mov ah,#0x88
    int 0x15
    mov [2], ax

! Video-card 데이터 받아오기
    mov ah,0x0f
    int 0x10
    mov [4],bx ! bh = 디스플레이 페이지
    mov [6],ax ! al = video mode, ah = window width

! EGA/VGA 설정 파라미터 체크하기
    mov ah,#0x12
    mov bl,#0x10
    int 0x10
    mov [8],ax
    mov [10],bx
    mov [12],cx

! hd0 data 가져오기
    mov ax,#0x0000
    mov ds,ax
    lds si,[4*0x41]
    mov ax, #INITSEG
    mov es,ax
    mov di,#0x0080
    mov cx,#0x10
    rep
    movsb

! hd1 data 가져오기
    mov ax,#0x0000
    mov ds,ax
    lds si,[4*0x46]
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0090
    mov cx,#0x10
    rep
    movsb

! hd1이 있는지 체크하기
    mov ax,#0x1500
    mov dl,0x81
    int 0x13
    jc no_disk1
    cmp ah,#3
    je is_disk1
no_disk1:
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0090
    mov cx,#0x10
    mov ax,#0x00
    rep
    stosb
is_disk1:
! 이제 우리는 보호 모드로 전환하길 원해.
    cli ! 인터럽트를 비활성화하고, OS가 지정하는 인터럽트 루틴을 사용하겠다고 컴퓨터에 선언하기
! 먼저 시스템을 올바른 곳으로 이동.
    mov ax,#0x0000
    cld
do_move:
    mov es,ax !목적지 세그먼트
    add ax,#0x1000
    cmp ax,#0x9000
    jz end_move
    mov ds,ax !소스 세그먼트
    sub di,di
    sub si,si
    mov cx,#0x8000
    rep
    movsw
    jmp do_move

! 그리고 우린 세그먼트 디스크립터를 로딩한다.
end_move:
    mov ax,#SETUPSEG
    mov ds,ax
    lidt idt_48 ! load idt with 0.0
    lgdt gdt_48 ! load gdt with whatever appropriate

!that was painless now we enable a20
    call empty_8042
    mov al,#0xD1 !write command
    out #0x64,al
    call empty_8042
    mov al,#0xDF ! A20 핀 켜버리기
    out #0x60, al
    call empty_8042

!인터럽트 메커니즘을 위해 PIC 8259A 재프로그래밍
    mov al,#0x11 ! 시퀀스 초기화
    out #0x20, al ! 8259A-1로 보내기
    .word 0x00eb, 0x00eb ! jmp$+2, jmp$+2
    out #0xA0, al ! 그리고 8259A-2로 보내기
    .word 0x00eb, 0x00eb
    mov al,#0x20 ! 하드웨어 인터럽트 0x20
    out #0x21,al
    .word 0x00eb, 0x00eb
    mov al,#0x28 ! 하드웨어 인터럽트 0x28
    out #0xA1, al
    .word 0x00eb, 0x00eb
    mov al,#0x04 ! 8259-1 is master
    out #0x21,al
    .word 0x00eb, 0x00eb
    mov al,#0x02 ! 8259-2 is slave
    out #0xA1,al
    .word 0x00eb, 0x00eb
    mov al, #0x01
    out #0x21,al
    .word 0x00eb, 0x00eb
    out #0xA1, al
    .word 0x00eb, 0x00eb
    mov al,#0xFF ! mask of all interrupts for now
    out #0x21, al
    .word 0x00eb, 0x00eb
    out #0xA1, al

! 보호 모드 전환 하기
    mov ax,#0x0001 ! 보호 모드 비트 켜버리기
    lmsw ax ! this is it
    jmpi 0,8 ! jmp offset 0 of segment 8 (cs)

empty_8042:
    .word 0x00eb, 0x00eb
    in al, #0x64 ! 8042 status port
    test al,#2 ! is input buffer full?
    jnz empty_8042 ! yest - loop
    ret

gdt:
    .word 0,0,0,0 !dummy

    .word 0x07FF ! 8Mb
    .word 0x0000 ! base address=0
    .word 0x9A00 ! code /읽기/실행
    .word 0x00C0 ! 세분성=4096??

    .word 0x07FF
    .word 0x0000
    .word 0x9200 ! data /읽기/쓰기
    .word 0x00C0
idt_48:
    .word 0 !idt limit = 0
    .word 0,0 !idt base = 0 초기값 설정

gdt_48:
    .word 0x800 !gdt limit 2048, 256 gdt entries
    .word 512+gdt,0x9 ! gdt base = 0x9xxx
.text
endtext:
.data
enddata:
.bss
endbss:
