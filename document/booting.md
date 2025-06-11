## 부팅 과정

컴퓨터에 전원이 켜진 순간에 DRAM에는 아무것도 들어있지 않다.
플로피 디스크에 OS가 저장되어 있을 뿐이다.
CPU는 메모리에 있는 프로그램만 실행할 수 있기 때문에, 컴퓨터가 플로피에 있는 OS를 직접 실행시킬 순 없다. OS를 동작시키기 위해서는, 플로피 디스크에서 메모리로 OS를 로드해야 한다.

램에 OS가 없다면 그것을 로드해주는 역할은 바로 **BIOS**가 하게 된다.

BIOS는 메인보드에 있는 작은 롬(ROM, 읽기만 가능한 메모리) 칩에 저장된다.
CPU가 리셋되면, CS:IP = 0xFFFF0으로 강제로 설정하게 되기 때문에, 
0xFFFF0에 있는 명령을 실행하게 된다.
그 위치에 명령이 없다면, 컴퓨터는 동작하지 않는다.

SeaBIOS의 다음 코드는 0xFFFF0에 지시가 넘어갈 때, BIOS Entry Code로 넘어가는 과정을 구현하고 있다.

```
// src/romlayout.S
entry_iret_official:
        iretw

        ORG 0xff54
        IRQ_ENTRY_ARG 05

        ORG 0xfff0 // Power-up Entry Point
        .global reset_vector
reset_vector:
        ljmpw $SEG_BIOS, $entry_post

        // 0xfff5 - BiosDate in misc.c

        // 0xfffe - BiosModelId in misc.c

        // 0xffff - BiosChecksum in misc.c

        .end
```

여기서 ORG asm은 뒤따르는 레이블 (reser_vector) 가 물리 주소 0xFFFF0에 위치할 수 있도록 어셈블된 이미지 상의 오프셋을 맞추는 과정이다.
이러한 과정을 통해 SeaBIOS는 플로피 디스크에서 운영체제를 0xFFFF0에 올려둘 수 있게 된다.

BIOS는 메모리의 처음 1KB 영역(0x00000~0x003FF)에 인터럽트 벡터 테이블을 위치시키고,
그 다음 256바이트 영역에(0x00400~0x004FF) BIOS 데이터를 위치시킨다.
그리고 인터럽트 서비스 루틴 (8KB 크기)는 앞에서 56KB 이후인 0xE05B에 로드된다.

여기서 인터럽트 벡터 테이블은 256개씩 인터럽트 벡터를 저장하고, 각각의 벡터마다 4바이트가 할당되는데, CS와 IP를 위하여 각각 2바이트씩 할당한다. 각각의 인터럽트 벡터는 특별한 인터럽트 루틴을 가리킨다.

이후 BIOS는 INT 0x19h를 실행하게 되고, 플로피 디스크의 **첫 번째 섹터**인 부트섹트를 메모리에 로드하게 되낟.
그리고 나서 앞서 로드했던 부트섹트가, 다시 OS의 두 번째, 세 번째 파트를 메모리에 로드한다.
이것은 각각 4섹터, 240섹터를 차지한다. 
(이것은 리눅스 커널 0.11의 과정이고, 초기 lectyos의 크기는 더 작을 수 있다.)

BIOS가 실행한(메모리에 올려 둔) INT 0x19h를 받게 CPU가 받게 되면, 
인터럽트 벡터 테이블에서 해당 벡터를 찾아, CS:IP를 0x0E6F2로 변경한다.

BIOS의 역할은 디바이스의 첫 번째 섹터를 읽어, 메모리에 저장한다. 그것이 역할의 끝이다.

OS와 BIOS의 상호 관례가 있다.

BIOS의 관례는 부트섹터를 메모리의 0x07C00에 올려둬야 된다는 것이고,
OS의 관례는 0번 트랙의 첫 번째 섹터에 부트섹터를 넣어야 한다는 것이다.

이를 둘 다 지키기 때문에, SeaBIOS는 0x07C00에 lectyos bootsect를 올려둘 것이다. lectyos가 해야 할 일은 bootsect.S 코드를 플로피 디스크의 0번 트랙 첫 번째 섹터에 부트섹터를 넣어두는 것이다.

리얼 모드에서 최대 사용 가능한 메모리는 1MB이다. 메모리 배치를 위해서 부트섹트는 다음 명령을 먼저 실행한다.
```
SETUPLEN = 4 ! 셋업 섹터는 4개에요!
BOOTSEG = 0x07c0 ! 부트 섹터는 여기에 로드 돼요
INITSEG = 0x9000 ! 부트 섹터를 여기로 옮겨요
SETUPSEG = 0x9020 ! SETUP은 여기서 시작돼요
SYSSEG = 0x1000 ! 시스템은 0x10000 (65336)에 로드돼요!
ENDSEG = SYSSEG + SYSSIZE ! 0X10000 + SYSSIZE만큼만 로드하고 로딩 멈춰요
```

관례에 따라 해당 bootsect 코드는 메모리 0x07C00에 위치하게 되고,
cpu cs:ip를 0x07C00으로 옮겨두는 것 까지 모두 OS가 담당하게 된다.

```
entry start
start:
    mov ax,#BOOTSEG
    mov ds,ax
    mov ax,#INITSEG
    mov es,ax
    mov cx,#256
    sub si,si
    sub di,di
    rep
    movw
    jmpi go, INITSEG
```
해당 코드는, 512바이트 크기인 자기 자신을 0x90000으로 복사한다.
한 워드는 2바이트이며, mov cx, #256은 256회 반복한다는 뜻이다.
그리고 CS를 initseg = 0x9000으로 이동하게 된다.
이 동작은, OS가 메모리를 내부에서 설계한 요구사항대로 재배치하는 것이다.


```
go:
    mov ax,cs
    mov ds,ax
    mov es,ax
! 스택을 0x9FF00부터 0x90000까지 쓸 수 있게 설정해둬요
    mov ss,ax
    mov sp, #0xFF00 

```

0x07C00은 상호 관례를 위한 것이지, 이 때부터 OS는 BIOS와 별개로 동작하게 된다.

이 때, CS:IP는 go:mov ax,cs를 가리키게 된다.

이후 ds,es,ss,sp와 같은 다른 세그먼트들을 0x90000으로 변경하는 작업을 한다.

push와 pop 명령을 사용하려면 ss와 sp가 정의되어 있어야 한다.
ss=0x9000
sp=0xFF00
으로 하면, ss:sp는 0x9FF00이 된다. >>512는 적절한 크기이다.

이후 setup.s를 로드한다.

```
load_setup:
    mov dx,#0x0000 ! drive 0, head 0
    mov cx,#0x0002 ! sector 2, track 0
    mov bx,#0x0200 ! address = 512 in INITSEG
    mov ax,#0x0200+SETUPLEN ! service 2, 섹터의 수
    int 0x13
    jnc ok_load_setup ! ok - continue
    mov dx,#0x0000 
    mov ax,#0x0000 ! 플로피디스크 리셋하기
    int 0x13
    j load_setup
```
이 코드는 setup.s를 읽어오는 코드이다.
dx=0은 0번 드라이브의 0번 헤드를 의미하고,
cx=2는 섹터2 부터, 트랙 0번을 읽어오라는 뜻이다.
bx=0x0200은 address = 512번, initseg로부터 512만큼 떨어진, 0x90200에 저장하라는 뜻이다.
ax=0x0200+setuplen은 service 2, 섹터는 4개. 를 가져오라는 뜻이다.
그리고 int 0x13h를 호출하게 되면, 0x90200으로 섹터 2부터의 데이터를 로드할 수 있다. (여기서 섹터 2부터의 위치에는 setup.s가 들어있다.)

! 디스크 드라이브 파라미터를 받아와요.
    mov dl,#0x00
    mov ax,#0x0800 ! AH=8은 디스크 파라미터를 의미해요
    int 0x13
    mov ch,#0x00
    seg cs
    mov sectors, cx
    mov ax,#INITSEG
    mov es,ax
이번엔 int 0x13h를 통해 디스크 파라미터를 받아와서, 
세그먼트 변수 sectors에 cx의 값을 대입하고
ax에 initseg값 0x90000을 넣는다.
그리고 es에도 동일한 값을 세팅한다.

! 여기서 메세지를 출력해요
    mov ah,#0x03 ! 커서의 포지션 읽어오기
    xor bh,bh
    int 0x10

    mov cx,#29 ! 문자열 길이는 24
    mov bx,#0x0007 ! page 0, attribute 7(normal)
    mov bp,#msg1
    mov ax,#0x1301 ! 스트링을 입력하고 커서를 옮겨요
    int 0x10
! 메세지 입력 성공
이후 메세지를 출력한다.
여기서 msg1의 값은, 

msg1:
    .byte 13,10
    .ascii "LectyOS Mesage Test ..."
    .byte 13, 10, 13, 10

이다. ascii를 통해 아스키 코드를 메세지에 출력한다.
여기서 인터럽트 벡터 테이블에서 int 0x10을 호출하면 메세지를 출력할 수 있다는 단서를 얻을 수 있다.

이후 head.s의 크기에 맞는 240섹터(?)의 크기를 가진 

! 메세지 입력 성공
! 시스템 로딩 시작하기
    mov ax,#SYSSEG
    mov es,ax ! segment of 0x010000
    call read_it
    call kill_motor

여기서 시스템 로딩을 시작한다.
240섹터 분량의 크기의 리눅스 커널을 0x10000에 올려두는 작업인데,
이는 다음과 같이 진행된다.
read_it:
    mov ax,es
    test ax,#0x0FFF ! es must be at 64kB boundary
die: jne die
    xor bx,bx ! bx is starting address within segment

ax 레지스터가 0x0FFF <- 64KB 바운더리로 되어 있는지 확인하고,
아니라면 무한 루프를 돌게 된다.

rp_read:
    mov ax,es
    cmp ax,#ENDSEG
    jb ok1_read
    ret
es(현재 읽고 있는 위치)가, #ENDSEG(시스템의 크기) 보다 작으면 ok1_read로 점프,그렇지 않으면 리턴한다.

ok1_read:
    seg cs
    mov ax,sectors
    sub ax,sread
    mov cx,ax
    shl cx,#9
    add cx,bx
    jnc ok2_read
    je ok2_read
    xor ax,ax
    sub ax,bx
    shr ax,#9
    
해당 코드는 sectors 변수 = 디스크 드라이브 파라미터 중, sector/tracks를 가져왔으니, 
sread = 현재 읽은 섹터 수 를 뺀다.
cx에 ax를 옮기고,
left로 9칸 옮긴다. 여기서 left로 9칸을 옮기는 이유는, cx 레지스터의 값을 512배로 만드는 연산이 필요하기 때문이다.
디스크 읽기 루틴에서는, 섹터 수 x 512바이트를 해야 될 일이 많다.
섹터 개수 차이를 ax에 담은 뒤, cx로 옮겨 shl을 하면,
(sectors - sread) * 512 바이트가 되어, 버퍼 오프셋 계산이나 남은 바이트 수 체크등에 바로 활용할 수 있다.


ok2_read:
    call read_track

해당 코드는, read_track을 통해 
트랙을 읽어오는 역할을 한다.


read_track:
    push ax
    push bx
    push cx
    push dx
    mov dx,track
    mov cx,sread
    inc cx
    mov ch,dl
    mov dx,head
    mov dh,dl
    mov dl,#0
    and dx,#0x0100
    mov ah,#2
    int 0x13
    jc bad_rt
    pop dx
    pop cx
    pop bx
    pop ax
    ret
bad_rt:     mov ax,#0
    mov dx,#0
    int 0x13
    pop dx
    pop cx
    pop bx
    pop ax
    jmp read_track

read_track은 ax, bx, cx, dx 레지스터를 스택에 push해두고,
우리가 0x9FF00으로 스택을 설정해뒀기 때문에
스택을 저장할 수 있고,
ax,bx,cx,dx의 값을 스택에 저장해놓고,
dx에는 track의 값을,
dx는 현재 읽고 있는 섹터의 값을 가져온다.
cx의 값을 하나 올리고,
ch에 dl(dx의 하위 8비트)를 옮기고,
dx에 head의 정보를 저장한다.
그리고 dh에 dl(dx의 하위 8비트) 정보를 올리고
dl을 0으로 지정한다.
and dx #0x0100을 통해 dx의 8~11번 비트만 켜두고,
ah에 2를 올린 채로 int 0x13을 호출한다. 
(이 코드로 인해, 한 트랙의 한 섹터를 읽어올 수 있다.)
jc(오류가 발생하지 않으면)
pop ax,bx,cx,dx를 통해 저장해둔 레지스터를 팝해온다.

bad return 되었을 경우, ax와 dx를 0으로 바꿔서 다시 pop한후, read_track으로 점프한다. (다시 읽기)

mov cx,ax
	add ax,sread
	seg cs
	cmp ax,sectors
	jne ok3_read
	mov ax,#1
	sub ax,head
	jne ok4_read
	inc track

이후의 코드는 read_track이 성공했을 얘기이다.
ax를 cx에 옮기고,
ax에 sread를 더한다.
ax와 sectors를 저장하고,
not equals라면 ok3_read로 넘어간다.
equals라면 
ax에 1을 넣고,
ax에 head를 빼고,
not equals라면 ok4_read로 넘어간다.
track을 1 올린다.

ok4_read:
	mov head,ax
	xor ax,ax
ok3_read:
	mov sread,ax
	shl cx,#9
	add bx,cx
	jnc rp_read
	mov ax,es
	add ax,#0x1000
	mov es,ax
	xor bx,bx
	jmp rp_read

이후의 코드는 정해진 track과 head, sector에 따라 읽어온다.

! root-device 찾기
    seg cs
    mov ax, root_dev
    cmp ax,#0
    jne root_defined
    seg cs
    mov bx,sectors
    mov ax, #0x0208 ! /dev/at1
    cmp bx,#15
    je root_defined
    mov ax,#0x021c ! /dev/ps0
    cmp bx,#18
    je root_defined
undef_root:
    jmp undef_root
root_defined:
    seg cs
    mov root_dev,ax

    jmpi 0,SETUPSEG

루트 디바이스를 찾은 후 setup seg로 점프한다.