#include "bootpack.h"

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* enable mouse--becasue in that age, mouse is a fast device for OS , default is disable */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0; // init by the way
	return; /* ACK(0xfa) */
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {//wait
		if (dat == 0xfa) {//like request message, just activate to get info
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* wait for the first byte and get it */
		if ((dat & 0xc8) == 0x08) { // confirm
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* wait for the second byte and get it */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* wait for the third byte and get it */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* a setting, video y and the y value is reverse */
		return 1;
	}
	return -1; /*  error */
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