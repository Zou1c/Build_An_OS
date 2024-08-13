/* to tell C compiler there are functions built with other files*/
#include "bootpack.h"
#include <stdio.h>

struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

extern struct FIFO8 keyfifo, mousefifo;

void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);

void HariMain(void)
{

	char *vram;
	int xsize, ysize , i;
	struct BOOTINFO *binfo; 
	
	char mcursor[256];
	char s[40];
	char keybuf[32] ,mousebuf[1024];
	int mouse_x, mouse_y;
	struct MOUSE_DEC mdec;

	binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	xsize = (*binfo).scrnx; //=base+4
	ysize = (*binfo).scrny; //=base+6
	vram = (*binfo).vram;	//=base+4+2*2

	init_gdtidt();
	init_pic();
	io_sti();  // the end of IDT/PIC initialization，open the interrupt of CPU
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 1024, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* allow interrupts from PIC1 and keyboard(11111001) */
	io_out8(PIC1_IMR, 0xef); /* allow interrupts from mouse(11101111) */
	
	init_palette();

	init_screen8(vram, xsize, ysize);
	mouse_x = (binfo->scrnx - 16) / 2; /* caculate in the center of screen */
	mouse_y = (binfo->scrny - 28 - 16) / 2;
	init_keyboard();

	init_mouse_cursor8(mcursor, COL8_008484); 
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 80, 80, mcursor, 16); 

	// extern char hankaku[4096];
	// putfont8(binfo->vram, binfo->scrnx, 128, 128, COL8_FFFFFF, hankaku + 'A' * 16);
	// putfonts8_asc(vram, xsize, 100, 100, COL8_848484, "hello, string!");


	enable_mouse(&mdec);

	for (;;) { 
		io_cli(); 
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) { // no interrupt but get request 
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} 
			else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);  // i means info
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mouse_x, mouse_y, mouse_x + 15, mouse_y + 15);// cover the origin mouse
					mouse_x += mdec.x;
					mouse_y += mdec.y;
					if (mouse_x < 0) {
						mouse_x = 0;
					}
					if (mouse_y < 0) {
						mouse_y = 0;
					}
					if (mouse_x > binfo->scrnx - 16) {
						mouse_x = binfo->scrnx - 16;
					}
					if (mouse_y > binfo->scrny - 16) {
						mouse_y = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); 
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); 
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mouse_x, mouse_y, mcursor, 16); 
				}
			}
		}
	} 
}
//io_cli //disable interrups
//...<your code>...
//io_sti //enable interrups

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

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

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

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
