
typedef unsigned int sigset_t; //32 bits

struct sigacation{
    void(*sa_handler)(int); //시그널 핸들러 함수 포인터
    sigset_t sa_mask; //핸들러 실행 중 블록할 시그널 집합(bitmask)
    int sa_flags; //핸들링 동작 제어용 플래그
    void(*sa_restorer)(void); //유저모드 복귀 시 사용되는 복원함수 포인터
}