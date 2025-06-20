#define MAJOR_NR 1
#include "blk.h"
#include <linux/fs.h>
char *rd_start;
int rd_length = 0;

void do_rd_request(void)
{
    int     len;
    char    *addr;

    INIT_REQUEST;
    addr = rd_start + (CURRENT->sector << 9);
    len = CURRENT->nr_sectors<<9;
    if((MINOR(CURRENT->dev) != 1)|| (addr+len > rd_start + rd_length)){
        end_request(0);
        goto repeat;
    }
    if(CURRENT->cmd == WRITE){
        (void)memcpy(addr,CURRENT->buffer,len);
    }else if(CURRENT->cmd==READ){
        (void)memcpy(CURRENT->buffer,addr,len);
    }else
        panic("unknown ramdisk command");
    end_request(1);
    goto repeat;
}

long rd_init(long mem_start, int length)
{
    int     i;
    char    *cp;

    blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
    rd_start = (char*)mem_start;
    rd_length = length;
    cp = rd_start;
    for(i=0;i<length;i++)
        *cp++='\0';
    return (length);
}

long rd_load(void)
{
    
}