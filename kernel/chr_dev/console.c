#include <asm/io.h>
#include <asm/system.h>

#define ORIG_X (*(unsigned char *)0x90000)
#define ORIG_Y (*(unsigned char *)0x90001)
#define ORIG_VIDEO_PAGE (*(unsigned short *)0x90004)
#define ORIG_VIDEO_MODE ((*(unsigned short *)0x90006)&0xff)
#define ORIG_VIDEO_COLS (((*(unsigned short *)0x90006)&0xff00)>>8)
#define ORIG_VIDEO_LINES (25)
#define ORIG_VIDEO_EGA_AX (*(unsigned short *)0x90008)
#define ORIG_VIDEO_EGA_BX (*(unsigned short *)0x9000a)
#define ORIG_VIDEO_EGA_CX (*(unsigned short *)0x9000c)

#define VIDEO_TYPE_MDA 0x10 /*모노크롬 텍스트 디스플레이*/
#define VIDEO_TYPE_CGA 0x11 /*CGA 디스플레이*/
#define VIDEO_TYPE_EGAM 0x20 /*EGA VGA를 모노크롬 모드에서 활용*/
#define VIDEO_TYPE_EGAC 0x21 /*EGA VGA를 컬러 모드에서 활용*/

#define NPAR 16

extern void keyboard_interrupt(void)

static unsigned char video_type; //사용하는 디스플레이의 종류
static unsigned long video_num_columns; //텍스트 컬럼의 수
static unsigned long video_size_row; //로우당 몇 바이트인지
static unsigned long video_num_lines; //텍스트 줄의 수는 몇인지
static unsigned char video_page; //비디오 페이지를 초기화
static unsigned long video_mem_start; //Video RAM의 시작부
static unsigned long video_mem_end; //Video RAM의 끝부분
static unsigned short video_port_reg; //port를 설정하는 video 레지스터
static unsigned short video_port_val; //value를 받아오는 video reg
static unsigned short video_erase_char; //지우는 녀석?

static unsigned long origin; //EGA/VGA Fast scroll
static unsigned long scr_end; //fast scroll에 쓰임
static unsigned long pos;
static unsigned long x,y;
static unsigned long top,bottom;
static unsigned long state=0;
static unsigned long npar,par[NPAR];
static unsigned long ques=0;
static unsigned char attr = 0x07;

static inline void gotoxy(unsigned int new_x, unsigned int new_y)
{
    if(new_x>video_num_columns || new_y>= video_num_lines)
        return;
    x=new_x;
    y=new_y;
    pos=origin + y*video_size_row + (x<<1);
}

void con_init(void)
{
    register unsigned char a;
    char* display_desc = "????";
    char* display_ptr;

    video_num_columns = ORIG_VIDEO_COLS;
    video_size_row = video_num_columns * 2;
    video_num_lines = ORIG_VIDEO_LINES;
    video_page = ORIG_VIDEO_PAGE;
    video_erase_char = 0x0720;

    if(ORIG_VIDEO_MODE == 7) //모노크롬 디스플레이인가?
    {
        video_mem_start = 0xb0000;
        video_port_reg = 0x3b4;
        video_port_val = 0x3b5;
        if((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type=VIDEO_TYPE_EGAM; //EGA 모노크롬 타입
            video_mem_end = 0xb8000;
            display_desc = "EGAm";
        }
        else
        {
            video_type = VIDEO_TYPE_MDA; //MDA 모노크롬 타입
            video_mem_end = 0xb2000;
            display_desc = "*MDA";
        }
    }
    else
    {
        video_mem_start = 0xb8000;
        video_port_reg = 0x3d4;
        video_port_val = 0x3d5;
        if((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type = VIDEO_TYPE_EGAC; //EGA 컬러 타입
            video_mem_end = 0xbc000;
            display_desc = "EGAc";
        }
        else
        {
            video_type = VIDEO_TYPE_CGA; //VGA 컬러 타입
            video_mem_end = 0xba000;
            display_desc = "*CGA";
        }
    }
    //이 부분에서 유저는 우리가 어떤 디스플레이 드라이버를 운용하는지 알게 된다.
    // [E G A c ]의 형태로 2바이트당 1문자씩 채운다
    display_ptr = ((char *)video_mem_start) + video_size_row - 8;
    while(*display_desc)
    {
        *display_ptr++ = *display_desc++;
        display_ptr++;
    }
    //스크롤에 쓰이는 변수 초기화
    origin = video_mem_start;
    scr_end = video_mem_start + video_num_lines * video_size_row;
    top=0;
    bottom = video_num_lines;

    gotoxy(ORIG_X,ORIG_Y);
    // 0x21=33=키보드 인터럽트 등록
    set_trap_gate(0x21,&keyboard_interrupt);
    //1111 1101 irq 1번 활성화
    outb_p(inb_p(0x21)&0xfd,0x21);
    a = inb_p(0x61); //키보드 설정값 저장
    outb_p(a|0x80,0x61); //키보드 nmi값 비활성화
    outb(a,0x61); //저장하여 다시 활성화함

}