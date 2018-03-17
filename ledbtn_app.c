#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/kdev_t.h>
/* 사용할 장치 파일의 이름이 변경됨 */
#define _MORSE_PATH_ "/dev/ledbtn"
 
int main(int argc, char *argv[]){
    int fd = 0;
 
    if((fd = open( _MORSE_PATH_, O_RDWR | O_NONBLOCK)) < 0){
        perror("open()");
        exit(1);
    }
    printf("open sungkong!\n"); 
 
    char a;// 모듈파일 파라미터때문에 필요한것. 모듈파일 수정필요 
     
    /* 장치 파일에 넘겨받은 인자로 쓰기 시도 */
    unsigned int btn=16; 

    while(read(fd, &btn, 1))
    {
//		write(fd,&a,1); 
	sleep(1);	
    }
	printf("button pushed.\n");
	write(fd,&a,1); 
	
    /* 쓰기가 끝나면 메시지 출력 */
    close(fd);
    return 0;
}
