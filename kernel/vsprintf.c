#include <stdarg.h>
#include <string.h>

//문자열 0~9의 경우에는 true를, 아님 false를 반환한다.
#define is_digit(c) ((c)>='0' && (c)<='9')

#define ZEROPAD     1
#define SIGN        2
#define PLUS        4
#define SPACE       8
#define LEFT        16
#define SPECIAL     32
#define SMALL       64

#define do_div(n,base) ({\
int __res;\
__asm__("divl %4"\ /*eax와 edx 레지스터로 구성된 64비트 피연산자를 32비트 피연산자로 (%4)로 나누라는 명령
    연산 전 eax 레지스터에는 피제수의 하위 32비트, n이 들어있고,
    edx 레지스터에는 피제수의 상위 32비트로 0을 설정해 둔다.
    divl r/m32는 edx:eax를 r/m32값으로 나눈다는 의미이며,
    연산 후 eax에는 몫이 들어가고,
    edx에는 나머지가 들어간다.
    그 뜻은, edx:eax로 된 64비트 레지스터의 값(즉,n)을 base로 나누고,
    edx에 나머지를, eax에 몫을 저장한다는 뜻.*/
    :"=a" (n),"=d" (__res)\
    /*
    제약 1은 출력으로 eax 레지스터를 사용하며, 연산 후 eax 값을 n에 저장하라는 의미 =은 write only를 의미한다.
    제약 2는 출력으로 edx 레지스터를 사용하며, 연산 후 edx 값을 c변수 __res에 저장하라는 의미이다.
    따라서 divl 실행 후 EDX(나머지) __res에 저장된다.
    */
    :"0"(n),"1"(0),"r"(base));\
    /*0 제약은 =a 제약을 그대로 사용한다는 뜻으로, 입력에도 n을 사용한다는 뜻,
    1 제약은 =d 제약을 그대로 사용하고, 상위 32비트에 0을 로드하여 사용한다는 뜻이다.
    r 제약은 어떤 일반 레지스터에도 base값을 로드하여 사용할 수 있도록 하라는 의미이다.*/
__res; \
})

//너비 서식이 있을 경우 너비 서식 만큼 들였는 코드.
static int skip_atoi(const char **s)
{
    //만약에 string의 숫자가 '1'이라면,
    int i=0;
    // string의 숫자가 '25'이라면, i= 0+ '2'-'0' = 2가 되고,
    //s는 '5'가 된다.
    //'5'는 is_digit을 성립하므로
    //같은 방식을 통해 i=2*10+5가 된다. 이를 통해 문자열에서 int값을 얻어올 수 있다.
    while(is_digit(**s))
        i = i*10+*((*s)++) - '0';
    return i;
}
//수와 관련된 로직을 처리한다.
static char * number(char* str,int num,int base,int size,
    int precision,int type)
{
    char c,sign,tmp[36];
    const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;
    //type가 SMALL이라면 소문자 문자열 중에서 찾아서 쓴다.
    if(type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
    //type가 LEFT라면 ZEROPAD(=1)의 반전으로 둔다. (이유는 모르겠음)
    if(type&LEFT) type &= ~ZEROPAD;
    //base는 2이상 36이하이다.
    if(base <2 || base>36)
        return 0;
    //type&Zeropad == 0이라면, LEFT가 들어있거나 ZEROPAD가 꺼져 있는 상태
    //type&ZEROPAD == 1이라면, ZEROPAD 플래그가 켜져 있는 상태.
    c = (type & ZEROPAD) ? '0' : ' ';
    //SIGNBit가 켜져 있고 음수라면, 출력 시 '-' 문자와 number을 출력한다.
    if(type&SIGN && num<0){
        sign='-';
        num = -num;
    }else
        //PLUS 비트가 켜져 있다면, sign은 plus를, space bit가 켜져 있다면, space를, 아니면 0으로 지정한다.
        sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
    //sign이 0보다 크다면 size를 1 줄인다. <- 이건 space나 plus기호를 넣기 위해 한 칸 비워둔 것이다.
    if(sign) size--;
    //type = special이라면, base=16이거나 base=8인 상태인데, 8진수의 경우 -1[prefix=0], 16진수의 경우 -2 (prefix=0x) 
    if(type&SPECIAL)
        if(base == 16) size -=2;
        else if(base ==8) size --;
    i=0;
    //num이 0이라면, 문자열 0을 추가한다.
    if(num ==0)
        tmp[i++]='0';
    else while(num != 0)
        //num이 0일 때 까지, tmp 배열에 문자열 배열에서 알맞는 값을 골라 집어넣는다.
        //예를 들어, num이 65, base가 16이라면, 나머지인 1이 출력되고,
        //digits[1]은 1이므로 1이 들어간다.
        //배열이 Z까지 있는 이유는, 35진수까지 지원하기 때문이다.
        //num은 '인자'이기 때문에 꼭 캐릭터 하나일 필요는 없다.
        tmp[i++] = digits[do_div(num,base)];
    //i가 precision보다 크다면, precision은 i가 된다.
    if(i>precision) precision = i;
    //size에서 precision을 빼준다.
    size -= precision;
    //type에 ZEROPAD와 LEFT가 꺼져 있다면
    if(!(type&(ZEROPAD+LEFT)))
        while(size-->0)
            *str++ = ' '; //SPACE만큼을 출력하게 한다.
    if(sign)
        *str++=sign; //부호를 더해준다.
    if(type&SPECIAL)
        if(base==8)
            *str++='0'; //8진수라면 0을 붙히고,
        else if(base==16){
            *str++='0';
            *str++ = digits[33]; //16진수라면 0x를 붙힌다.
        }
    if(!(type&LEFT)) //LEFT가 켜져 있지 않다면, (ZEROPAD인 경우)
        while(size-->0)
            *str++=c; //c 문자는 ZEROPAD의 경우, 0이고, 아닐 경우 공백이다.
    while(i<precision--)
        *str++='0';  // i보다 precision이 크다면,(예측 문자열의 크기가 문자열의 길이보다 길다면) 0을 붙혀준다.
    while(i-->0)
        *str++ = tmp[i];  //임시 배열에 있는 값을 원 배열로 옮긴다.
    while(size -->0) //size가 0이 될 때 까지 공백을 출력한다. (공백 채우기.)
        *str++=' ';
    return str;

}

//printf 함수의 형식에는 %[flag][width][precision]type의 형태로 들어간다.
int vsprintf(char * buf, const char* fmt,va_list args)
{
    int len;
    int i;
    char * str;
    char * s;
    int * ip;
    int flags; //number() 함수의 flag를 의미한다.

    int field_width; // output fields의 width를 의미한다.
    int precision; //precision = min값은 정수의 디지털값, max는 string의 캐릭터의 수이다.

    int qualifier; //h, l,L의 integer fields를 의미한다.

    //buf를 str에 복사하고, fmt의 값을 ++해 나가는 과정.
    for(str = buf;*fmt;++fmt){
        //퍼센트 기호라면?
        if(*fmt != '%'){
            //스트링 포인터를 포맷 포인터랑 같이 ++해준다.
            //포맷 기호가 아니라면 그냥 그대로 출력하면 되기 때문.
            *str++ = *fmt;
            continue;
        }
        //프로세스 플래그.
        flags=0;
        repeat:
            ++fmt;
            switch(*fmt){
                //-가 오면 LEFT FLAG를 켠다. 플래그는 두 개 이상이 올 수 있다.
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
                }
        field_width = -1;
        //너비 서식값이 있는지 확인하는 코드.
        if(is_digit(*fmt))
                field_width = skip_atoi(&fmt);
        //만약 fnt가 *이라면, 
        else if(*fmt == '*'){
            //매개 변수를 하나 가져오고,
            field_width = va_arg(args,int);
            //매개 변수가 음수라면
            if(field_width<0){
                //너비 서식값을 양수로 바꾼 후 
                //Left 서식값을 지정한다.
                field_width=-field_width;
                flags |= LEFT;
            }
        }
        //precision은 출력할 최대 갯수를 명시한다.
        precision = -1;
        //.으로 시작하는 포맷이 있다면,
        if(*fmt == '.'){
            //한 칸 올리고
            ++fmt;
            //0~9사이인지 확인해서
            if(is_digit(*fmt))
                //숫자로 변환한다.
                precision = skip_atoi(&fmt);
                //또한 asterisk라면, 매개 변수에서 가져온다.
            else if(*fmt == '*'){
                precision = va_arg(args,int);
            }
            //pricision은 양수이다.
            if(precision <0)
                precision = 0
        }
        
        qualifier = -1;
        //모지? 코드 봐야 할 듯
        if(*fmt == 'h' || *fmt == 'l' || *fmt=='L'){
            qualifier = *fmt;
            ++fmt;
        }
        
        switch(*fmt){
            //출력할 캐릭터가 char이라면, precision을 무시한다.
            case 'c':
                if(!(flags & LEFT))
                    //field_width만큼 space <- LEFT 플래그가 켜져있으면 먼저 진행
                    while (--field_width>0)
                    {
                        *str++ = ' ';
                    }
                    //문자열을 str에 집어넣는다.
                    *str++ = (unsigned char)va_arg(args,int);
                    //field_width 만큼 space <- 안 켜져 있으면 나중에 진행
                    while(--field_width > 0)
                        *str++ = ' ';
                    break;
            case 's':
                    //출력할 캐릭터가 문자열이라면,
                    s = va_arg(args,char *);
                    len = strlen(s);
                    //precision이 음수라면(Pricision이 지정되지 않으면 -1이다.)
                    if(precision < 0)
                        precision = len; //precision을 len으로 바꿔둔다.
                    else if(len>precision)
                        len = precision; //len이 pricision보다 크다면, len을 precision만큼만 출력하도록 한다.
                    
                        //위와 동일
                        if(!(flags & LEFT))
                            while(len < field_width--)
                                *str++ = ' ';
                        for(i=0;i<len;++i)
                            *str++ = *s++;
                        while(len < field_width--)
                            *str++ = ' ';
                        break;
            case 'o':
                    //number함수를 적용한다.
                    str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
                    break;
            case 'p':
                    //포인터형으로 지정한 경우 field_width가 -1이면 8로 재설정한다.
                    //그리고 0을 넣어서 0x0000xxxx와 같은 형태로 출력하게 하고,
                    if(field_width==-1){
                        field_width = 8;
                        flags |= ZEROPAD;
                        
                    }
                    //16진수 형태를 가져온다. 이 때 주소를 가져와야 하므로 void ptr로 가져온다.
                    str = number(str, (unsigned long)va_arg(args,void *),16,field_width,precision,flags);
                    break;
            case 'x':
                    flags |= SMALL;
            case 'X':
                    //16진수
                    str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
                    break;
            case 'd':
            case 'i':
                    flags |= SIGN;
            case 'u':
                    //10진수 (u는 unsigned이다.)
                    str = number(str,va_arg(args,unsigned long),10,field_width,precision,flags);
                    break;
            case 'n':
                    //버퍼나 문자열에 기록된 개수를 해당 주소에 정수로 저장한다.
                    ip = va_arg(args,int *);
                    *ip = (str-buf);
                    break;
            default:
                    //format이 '%'가 아닌 경우, (해당 이외의 문자열이 들어왔을 경우에는, %으로 채운다.)
                    if(*fmt != '%')
                        *str++ = '%';
                    //format이 %인 경우, %%의 형태로 들어온 경우에는, %기호를 추가한다.
                    if(*fmt)
                        *str++ = *fmt;
                    //format이 0번 문자일 경우에는 --fmt를 진행해준다.
                    //근데 이 경우는 도달할 일이 없다.
                    else
                        --fmt;
                    break;
        }
            

    }
    //문자열의 마지막엔 \0으로 채워준다.
    *str = '\0';
    //str-buf를 하면 str의 시작 주소가 나오게 된다.
    return str-buf;
}