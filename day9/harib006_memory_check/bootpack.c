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

#define MEMMAN_FREES 4090 /* about 32KB*/
#define MEMMAN_ADDR 0x003c0000 
struct FREEINFO
{ /* can be allocated */
	unsigned int addr, size;
};
struct MEMMAN
{ /* memory management */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};//+

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

	unsigned int memtotal; //+
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;//+

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

	memtotal = memtest(0x00400000, 0xbfffffff); 
	memman_init(memman); 
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */ 
	memman_free(memman, 0x00400000, memtotal - 0x00400000); //+
	sprintf(s, "memory %dMB free : %dKB", 
			memtotal / (1024 * 1024), memman_total(memman) / 1024); 
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s); //+

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

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;	   /* 可用信息数目 */
	man->maxfrees = 0; /* 用于观察可用状况：frees的最大值 */
	man->lostsize = 0; /* 释放失败的内存的大小总和 */
	man->losts = 0;	   /* 释放失败次数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 报告空余内存大小的合计 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* 分配 */
{
	unsigned int i, a; // a means memory address(can be allocated)
	for (i = 0; i < man->frees; i++)
	{
		if (man->free[i].size >= size)
		{
			/* 找到了足够大的内存 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0)
			{
				/* 如果free[i]变成了0，就减掉一条可用信息 */
				man->frees--;
				for (; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1]; /* 代入结构体 */
				}
			}
			return a;
		}
	}
	return 0; /* 没有可用空间 */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 释放 */
{
	int i, j;
	/* 为便于归纳内存，将free[]按照addr的顺序排列 */
	/* 所以，先决定应该放在哪里 */
	for (i = 0; i < man->frees; i++)
	{
		if (man->free[i].addr > addr)
		{
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0)
	{
		/* 前面有可用内存 */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr)
		{
			/* 可以与前面的可用内存归纳到一起 */
			man->free[i - 1].size += size;
			if (i < man->frees)
			{
				/* 后面也有 */
				if (addr + size == man->free[i].addr)
				{
					/* 也可以与后面的可用内存归纳到一起 */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]删除 */
					/* free[i]变成0后归纳到前面去 */
					man->frees--;
					for (; i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1]; /* 结构体赋值 */
					}
				}
			}
			return 0; /* 成功完成 */
		}
	}
	/* 不能与前面的可用空间归纳到一起 */
	if (i < man->frees)
	{
		/* 后面还有 */
		if (addr + size == man->free[i].addr)
		{
			/* 可以与后面的内容归纳到一起 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功完成 */
		}
	}
	/* 既不能与前面归纳到一起，也不能与后面归纳到一起 */
	if (man->frees < MEMMAN_FREES){
		/* free[i]之后的，向后移动，腾出一点可用空间 */
		for (j = man->frees; j > i; j--){
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees){man->maxfrees = man->frees; /* 更新最大值 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功完成 */
	}
	/* 不能往后移动 */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失败 */
}