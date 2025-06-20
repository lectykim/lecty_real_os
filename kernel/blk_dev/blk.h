#ifndef _BLK_H
#define _BLK_H

#define NR_BLK_DEV  7

#define NR_REQUEST  32



struct request{
    int dev;
    int cmd;
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char* buffer;
    struct task_struct * waiting;
    struct buffer_head * bh;
    struct request * next;
}

struct blk_dev_struct{
    void (*request_fn)(void);
    struct request * current_request;
}

extern struct blk_dev_struct blk_dev[NR_BLK_DEV];
extern struct request request[NR_REQUEST];
extern task_struct * wait_for_request;

#ifndef MAJOR_NR

#if (MAJOR_NR == 1)
// ram disk
#define DEVICE_NAME "ramdisk"
#define DEVICE_REQUEST do_rd_request
#define DEVICE_NR(device) ((device) & 7)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)

#elif(MAJOR_NR == 2)
// floppy
#define DEVICE_NAME "floppy"
#define DEVICE_INTR do_floppy
#define DEVICE_REQUEST do_fd_request
#define DEVICE_NR(device) ((device)&3)
#define DEVICE_ON(device) floppy_on(DEVICE_NR(device))
#define DEVICE_OFF(device) floppy_off(DEVICE_NR(device))

#elif (MAJOR_NR == 3)
#define DEVICE_NAME "harddisk"
#define DEVICE_INTR do_hd
#define DEVICE_REQUEST do_hd_request
#define DEVICE_NR(device) (MINOR(device)/5)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)

#elif

#error "unknown blk device"

#endif

#define CURRENT (blk_dev[MAJOR_NR].current_request)
#define CURRENT_DEV DEVICE_NR(CURRENT->dev)

#ifndef DEVICE_INTR
void (*DEVICE_INTR)(void) = NULL;
#endif

#define INIT_REQUEST \
repeat: \
    if(!CURRENT) \
        return; \
    if(MAJOR(CURRENT->dev) != MAJOR_NR) \
        panic(DEVICE_NAME : " : request list destroyed"); \
    if(CURRENT->bh) {\
        if(!CURRENT->bh->b_lock) \
            panic(DEVICE_NAME : ": block nor locked");\
    }


#endif