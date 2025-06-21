long last_pid=0;

int copy_process(int nr,long ebp, long edi, long esi, long gs, long nen,
long ebx, long ecx, long edx,
long fs, long es, long ds,
long eip, long cs, long eflags, long esp, long ss)
{
    struct task_struct *p;
    int i;
    struct file *f;

    p=(struct task_struct *) get_free_page();
    if(!p)
        return -EAGAIN;
    task[nr] = p;
    *p = *current;
    p->state = TASK_UNINTERRUPTABLE;
    p->father = current->pid;
    p->counter = p->priority;
}

int find_empty_process(void)
{
    int i;

    repeat:
        if((++last_pid)<0) last_pid=1;
        for(i=0;i<NR_TASKS;i++)
            if(task[i] && task[i]->pid ==last_pid) goto repeat;
        for(i=1;i<NR_TASKS;i++)
            if(!task[i])
                return i;
    return -EAGAIN;
    
}