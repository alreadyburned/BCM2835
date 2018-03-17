#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/fs.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mm.h>


#define DEV_LEDBTN_MAJOR_NUMBER  230
#define DEV_LEDBTN_NAME          "gpio-ledbtn"

#define LED_GPIO18                18
#define LED_GPIO19                19
#define BTN_GPIO16		  16

#define INPUT			   0
#define OUTPUT			   1
#define ON			   1
#define OFF			   0

#define BCM2835_PERI_BASE               0x3f000000
#define GPIO_BASE  (BCM2835_PERI_BASE + 0x00200000)


#define GPLEVx 13
#define GPSETx  7    //0x001C
#define GPCLRx 10    // 0x0028
#define GPFENx 0x0058



/*
   0x 7E20 0000 ~ 0014 GPFSELx function select
   0x 7E20 001C ~ 0020 GPSETx  setvalue
   0x 7E20 0028 ~ 002C GPCLRx  clear
   0x 7E20 0058 ~ 005C GPFENx  falling edge
   0x 7E20 0034 ~ 0038 GPLEVx  pin level
*/
struct GpioRegisters
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1;
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
};




static int set_bits(
    volatile unsigned int *addr, 
    const unsigned int shift, 
    const unsigned int val, 
    const unsigned int mask)
{
    unsigned int temp = *addr;
     
    temp &= ~(mask << shift);
    temp |= (val & mask) << shift;
    *addr = temp;
 
    return 0;
}

static unsigned int get_bit(    volatile unsigned int *addr,  const unsigned int shift){

    unsigned int temp = *addr;
    return (temp >> shift)&0x01;
}



static int func_pin(
    const unsigned int pin_num,
    const unsigned int mode)
{
    volatile unsigned int *gpio =  (struct GpioRegisters*)ioremap(GPIO_BASE, sizeof(struct GpioRegisters));

    /* 하나의 레지스터에 10개의 GPIO에 대한 Funtion을 설정할 수 있기 때문*/
    unsigned int pin_bank = pin_num / 10;
 
    if(pin_num > 53) return -1;
    if(mode > 7) return -1;
 
    gpio += pin_bank;
    set_bits(gpio, (pin_num % 10) * 3, mode, 0x7);

    return 0;
}

static int set_pin(
    const unsigned int pin_num, 
    const unsigned int status)
{
    volatile unsigned int *gpio =  (struct GpioRegisters*)ioremap(GPIO_BASE, sizeof(struct GpioRegisters));
    unsigned int pin_bank = pin_num >> 5;  //pin_num/32
 
    if(pin_num > 53) return -1;
    if(status != OFF && status != ON) return -1;
 
    gpio += pin_bank;
 
    if(status == OFF){
        set_bits(
            /*  GPIO의 output을 clear하는 레지스터는 base로부터 0x28만큼 떨어진 곳에 존재 */
            gpio + GPCLRx, 
            /* 하나의 GPIO에 대한 output은 1개의 bit로 표현됨 */
            pin_num, ON, 0x1);
        set_bits(
            gpio + GPCLRx, 
            pin_num, OFF, 0x1);
    }
    else if(status == ON){
        set_bits( gpio + GPSETx, pin_num%32, ON, 0x1);
        set_bits(gpio + GPSETx, pin_num%32, OFF, 0x1);
            /*
               1(켜짐) -> 0(변화없음). 
	       즉 다음번에 상태바꿀때 미리 0으로 해두면 다시 건드릴 필요가 없다.
            */
    }
 
    return 0;
}

  
static ssize_t ledbtn_read(
    struct file *filp, 
    const unsigned int *buf, 
    size_t count, 
    loff_t *f_pos){
    volatile unsigned int *gpio =  (struct GpioRegisters*)ioremap(GPIO_BASE, sizeof(struct GpioRegisters));    
    unsigned int pin_bank = *buf/32;
    unsigned int result = get_bit(gpio+GPLEVx+pin_bank,*buf);    
    return result;
}

static int ledbtn_open(struct inode *inode, struct file *filp){ 
   func_pin(LED_GPIO18,OUTPUT);
   func_pin(LED_GPIO19,OUTPUT);
   func_pin(BTN_GPIO16,INPUT);

 //  set_pin(LED_GPIO18,ON);
   //set_pin(LED_GPIO19,ON);
    printk("[gpio-ledbtn] The device \"ledbtn\" opened.\n");
    return 0;
}

/* 장치 파일을 close() 하면 호출 될 함수 */
static int ledbtn_release(struct inode *inode, struct file *filp){
    printk("[gpio-ledbtn] The device \"ledbtn\" closed\n");

    return 0;
}



static ssize_t ledbtn_write(
    struct file *filp, 
    size_t count, 
    loff_t *f_pos)
{
    int i;
    for(i=4;i>0; i--){
	set_pin(LED_GPIO18,ON);
        set_pin(LED_GPIO19,ON);
        msleep(250);
	set_pin(LED_GPIO18,OFF);
        set_pin(LED_GPIO19,OFF);
	msleep(250);
    }
    set_pin(LED_GPIO18,OFF);
    set_pin(LED_GPIO19,OFF);

    return count;
}
static struct file_operations ledbtn_fops = {
    .owner      = THIS_MODULE,
    .open       = ledbtn_open,
    .release    = ledbtn_release,
    .write    = ledbtn_write,
    .read     = ledbtn_read
    
    //.ioctl    = ledbtn_ioctl,
};
 

/* 모듈을 insmod로 삽입할 때 호출되는 함수 */
static int ledbtn_init(void){
     printk("[gpio-ledbtn] ledbtn module inserted.\n");
    /* 모듈을 주번호와 이름, 파일 오퍼레이션을 가지고 커널에 등록하는 함수 */
    register_chrdev(DEV_LEDBTN_MAJOR_NUMBER, DEV_LEDBTN_NAME, &ledbtn_fops);
    return 0;
}
 
/* 모듈을 rmmod로 제거할 때 호출되는 함수 */
static void ledbtn_exit(void){
    printk("[gpio-ledbtn] ledbtn_exit()\n");
    /* 주번호와 이름이 일치하는 모듈을 커널에서 등록 해제시키는 함수 */
    unregister_chrdev(DEV_LEDBTN_MAJOR_NUMBER, DEV_LEDBTN_NAME);
 
}
 
/* 모듈이 삽입될 때 morse_init()이 호출되도록 함 */
module_init(ledbtn_init);
/* 모듈이 제거될 때 morse_exit()이 호출되도록 함 */
module_exit(ledbtn_exit);
 
MODULE_LICENSE("Dual BSD/GPL");
