// #include <etc/simple_print.h>

// static void simple_print(const char* str)
// {
//     while(*str){
//         char c = *str++;
//         if(c == '\n'){
//             cursor_col = 0;
//             cursor_row++;
//         } else if(c =='\r'){
//             cursor_col = 0;
//         }else {
//             unsigned short offset = (unsigned short)(cursor_row * SCREEN_COLS + cursor_col);
//             vga_buffer[offset] = (unsigned short)c | ((unsigned short)VGA_DEFUALT_ATTR<<8);
//             cursor_col++;
//             if(cursor_col>=SCREEN_COLS){
//                 cursor_col=0;
//                 cursor_row++;
//             }
//         }
//     }
//     move_hardware_cursor();
// }