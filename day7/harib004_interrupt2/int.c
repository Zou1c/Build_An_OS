#include "bootpack.h"

#define PORT_KEYDAT 0x0060 

struct FIFO8 keyfifo;

void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* disable all interrups , PIC0 */
	io_out8(PIC1_IMR,  0xff  ); /* disable all interrups , PIC1 */

	io_out8(PIC0_ICW1, 0x11  ); /* （edge trigger mode） */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 will be received by AINT20-27 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 is connected by IRQ2 to PIC0 */
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* （edge trigger mode） */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 will be received by AINT28-2f */
	io_out8(PIC1_ICW3, 2     ); /* PIC1 is connected by IRQ2 */
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 disable all interrups, except PIC1*/
	io_out8(PIC1_IMR,  0xff  ); /* 11111111  disable all interrups */

	return;
}

void inthandler21(int *esp)
/* 从PS/2键盘插入; 来自PS/2键盘的中断  */
{
	unsigned char data;

	io_out8(PIC0_OCW2, 0x61); /* 通知PIC"IRQ-01已经收到中断请求了，可以发数据了" */ 
	
	data = io_in8(PORT_KEYDAT);//device port
	fifo8_put(&keyfifo, data);
	return;
}


struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断  */
{
	unsigned char data;
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	io_out8(PIC1_OCW2, 0x64);	/* response to IRQ-12 in PIC1 */
	io_out8(PIC0_OCW2, 0x62);	/* response to IRQ-02 in PIC0 */
	return;
}

void inthandler27(int *esp)
//该中断由PIC初始化时产生
{
	io_out8(PIC0_OCW2, 0x67); /* 向PIC通知IRQ-07受理完成*/
	return;
}
