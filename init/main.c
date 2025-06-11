#include <asm/io.h>

#define VGA_TEXT_BUF (volatile unsigned char *)0xB8000
#define SCREEN_COLS 80
#define SCREEN_ROWS 25
#define SCREEN_CELLS (SCREEN_COLS * SCREEN_ROWS)
#define VGA_DEFUALT_ATTR 0x07

static unsigned char cursor_row =0;
static unsigned char cursor_col = 0;
long user_stack [ 4096>>2 ] ;
struct {
	long * a;
	short b;
	} stack_start = { & user_stack [4096>>2] , 0x10 };
static volatile unsigned short* const vga_buffer = (volatile unsigned short*)VGA_TEXT_BUF;


static unsigned short get_cursor_pos_bare(void)
{
    unsigned short pos;
    unsigned char high,low;

    outb_p(0x0E,0x3D4);
    high = inb(0x3D5);
    outb_p(0x0F,0x3D4);
    low = inb(0x3D5);
    pos =((unsigned short) high <<8) | (unsigned short)low;
    return pos;
    
}

static void read_vga_chars_bare(char * out_buf,unsigned int count)
{
    volatile unsigned char *vid = VGA_TEXT_BUF;
    unsigned int i;

    if(count>SCREEN_CELLS)
        count = SCREEN_CELLS;

        for(i=0;i<count;i++){
            out_buf[i] = vid[i*2]; //ASCII
        }
}

static void move_hardware_cursor(void)
{
    unsigned short pos = (unsigned short)(cursor_row * SCREEN_COLS + cursor_col);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // high byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void simple_print(const char* str)
{
    while(*str){
        char c = *str++;
        if(c == '\n'){
            cursor_col = 0;
            cursor_row++;
        } else if(c =='\r'){
            cursor_col = 0;
        }else {
            unsigned short offset = (unsigned short)(cursor_row * SCREEN_COLS + cursor_col);
            vga_buffer[offset] = (unsigned short)c | ((unsigned short)VGA_DEFUALT_ATTR<<8);
            cursor_col++;
            if(cursor_col>=SCREEN_COLS){
                cursor_col=0;
                cursor_row++;
            }
        }
    }
    move_hardware_cursor();
}



void main(void)
{

    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    simple_print("Hello LectyOS !! \n");
    while(1){
            
    }
}