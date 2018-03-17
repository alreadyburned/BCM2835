#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
//------------ 이 위가 대부분의 드라이버에 사용되는 헤더--------------------------------------

#include <linux/string.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/scatterlist.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <mach/platform.h>
#include <mach/dma.h>

/*
PWM을 18번 핀으로 LED 켜는 시나리오
PWM 18-> ALT5번이 pwm0출력임 PWM의 채널1이 pwm0라고 가정 --> 아직 어느것인지 확인하지 않았음 추후 수정필요
1. PWM_CTL reg
(31:16)        (15 MSEN2)(14) (13 USEF2) (12 POLA2) (11 SBIT2) (10 RPTL2) (9 MODE2) (8 PWEN2) (7 MSEN1) (6 CLRF1) (5 USEF1) (4 POLA1) (3 SBIT1) (2 RPTL1) (1 MODE1) (0 PWEN1)
xxxxxxxxxxxxxxx    x       x      x          x          x            x         x     x          0           0         1     0         0      1           0          1

*/

static uint8_t servo2gpio[] = {
		4,	// P1-7
		17,	// P1-11
		18,	// bcm18 핀 PWM0. 원본은 PWM0_ON_GPIO18가 정의된 경우 1, 아니면 18
	  //21,	// P1-13 REV_1 
		27,	// P1-13 REV_2
		22,	// P1-15
		23,	// P1-16
		24,	// P1-18
		25,	// P1-22
};// NUM_SERVOS = 8?


static int dev_open(struct inode *, struct file *);
static int dev_close(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);


module_init( PWM_servo_init );

module_exit( PWM_servo_exit );



static int PWMservo_init(void);

static int PWMservo_exit(void);



static int PWMservo_init()

{
	res = alloc_chrdev_region(&devno, 0, 1, "servoblaster");
	//alloc_chrdev_region  : 디바이스 넘버 커널에 등록 
	//egister_chrdrv_region 함수는 원하는 디바이스의 번호를 미리 알고 있을 때 사용하고, 
	//alloc_chrdev_region 함수는 디바이스의 번호를 동적으로 할당받아 파라미터로 받는 dev_t 구조체 포인터를 이용해 dev_t 구조체에 넣는다.

	if (res < 0) {
		printk(KERN_WARNING "ServoBlaster: Can't allocated device number\n");
		return res;
	}

	my_major = MAJOR(devno);//주 번호값 반환


}



static int PWMservo_exit()
{
}

