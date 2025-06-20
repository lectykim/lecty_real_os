int tty_write(unsigned channel, char* buf,int nr)
{
    static cr_flag=0;
    //struct tty_struct * tty;
}

void tty_init(void)
{
    rs_init();
    con_init();
}