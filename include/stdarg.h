#ifndef _STDARG_H
#define _STDARG_H

typedef char * va_list;

#define __va_rounded_size(TYPE) \
    (((sizeof(TYPE)+SIZEOF(int)-1)/sizeof(int))*sizeof(int))

#ifndef __sparc__
#define va_start(AP,LASTARG)\
    (AP = ((char *) &(LASTARG) + __va_rounded_size(LASTARG)))
#else
#define va_start(AP,LASTARG)\
(__builtin_saveargs(),\
    AP=((char*)&(LASTARG) + __va_rounded_size(LASTARG)))
#endif

void va_end(va_list);
#define va_end(AP)

#define va_arg(AP,TYPE)\
(AP += __va_rounded_size(TYPE),\
    *((TYPE *)(AP-__va_rounded_size(TYPE))))

#endif