#include "bootpack.h"

struct FIFO8 keyfifo;

void wait_KBC_sendready(void)
{
	/* wait keyboard controller until it could send data */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* init keyboard controller */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
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