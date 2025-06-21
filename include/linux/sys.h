extern int sys_setup();
extern int sys_exit();
extern int sys_fork();

fn_ptr sys_call_table[] = {sys_setup,sys_exit,sys_fork}