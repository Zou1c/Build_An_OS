/* to tell C compiler there are functions built with other files*/
#include "bootpack.h"
#include <stdio.h>

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

	unsigned int memtotal; 
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

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
	
	//memory check
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024); 
	sprintf(s, "memory %dMB", i); 
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	memtotal = memtest(0x00400000, 0xbfffffff); 
	memman_init(memman); 
	memman_free(memman, 0x00001000, 0x0009e000); /* 620KB */ 
	memman_free(memman, 0x00400000, memtotal - 0x00400000); 
	sprintf(s, "memory %dMB free : %dKB", 
			memtotal / (1024 * 1024), memman_total(memman) / 1024); 
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s); 


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
