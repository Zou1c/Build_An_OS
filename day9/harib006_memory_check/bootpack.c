/* to tell C compiler there are functions built with other files*/
#include "bootpack.h"
#include <stdio.h>

#define EFLAGS_AC_BIT 0x00040000 
#define CR0_CACHE_DISABLE 0x60000000 

unsigned int memtest(unsigned int start, unsigned int end) 
{ //what you need
  //memtest_sub(), 
  //load_cr0(),store_cr0()

	char flg486 = 0; 
	unsigned int eflg, cr0, i;
	/* 确认CPU是386还是486以上的 */ 
	eflg = io_load_eflags(); 
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 , 一个判断位*/ 
	io_store_eflags(eflg); 
	eflg = io_load_eflags(); 
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */ 
		flg486 = 1; 
	} 
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */ 
	io_store_eflags(eflg); 

	if (flg486 != 0) { 
		cr0 = load_cr0(); 
		cr0 |= CR0_CACHE_DISABLE; /* 禁用缓存 */ 
		store_cr0(cr0); 
	} 
	i = memtest_sub(start, end); //实际的内存检查,从start到end范围内的
	if (flg486 != 0) { 
		cr0 = load_cr0(); 
		cr0 &= ~CR0_CACHE_DISABLE; /* 启用缓存 */ 
		store_cr0(cr0); 
	} 
	return i; 
}


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
	
	//memory check
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024); 
	sprintf(s, "memory %dMB", i); 
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

