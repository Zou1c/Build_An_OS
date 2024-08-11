/* to tell C compiler there are functions built with other files*/
#include <stdio.h>
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_ascii_str(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);//+
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

struct BOOTINFO
{
	char cyls, leds, vmode, reserve; // every occupy 1 byte
	short scrnx, scrny;				 // every occupy 2 bytes
	char *vram;						 // 4bytes
};

struct SEGMENT_DESCRIPTOR {//+
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {//+
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);//+
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);//+
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);//+
void load_gdtr(int limit, int addr);//+
void load_idtr(int limit, int addr);//+

void init_mouse_cursor8(char *mouse, char bc)
/* prepare mouse pointer（16×16） */
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"};
	int x, y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {mouse[y * 16 + x] = COL8_000000;}
			if (cursor[y][x] == 'O') {mouse[y * 16 + x] = COL8_FFFFFF;}
			if (cursor[y][x] == '.') {mouse[y * 16 + x] = bc; // background color
			}
		}
	}
	return;
}
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {
// copy the data in buf(address) to vram(address) 
//bxsize means buf's xsize(?)
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

void HariMain(void)
{
	init_gdtidt();

	char *vram;
	int xsize, ysize;
	struct BOOTINFO *binfo; //+

	init_palette();

	binfo = (struct BOOTINFO *)0x0ff0;
	xsize = (*binfo).scrnx; //=base+4
	ysize = (*binfo).scrny; //=base+6
	vram = (*binfo).vram;	//=base+4+2*2
	init_screen(vram, xsize, ysize);

	char mcursor[256];
	init_mouse_cursor8(mcursor, COL8_008484); 
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 80, 80, mcursor, 16); 

	extern char hankaku[4096];
	putfont8(binfo->vram, binfo->scrnx, 128, 128, COL8_FFFFFF, hankaku + 'A' * 16);

	putfonts8_ascii_str(vram, xsize, 100, 100, COL8_848484, "hello, string!");

	for (;;)
	{
		io_hlt();
	}
}

void putfonts8_ascii_str(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for (; *s != 0x00; s++)
	{
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{ // char c means "color"
	int i;
	char *memory_pointer, value;
	for (i = 0; i < 16; i++)
	{
		memory_pointer = vram + (y + i) * xsize + x;
		value = font[i];
		if ((value & 0x80) != 0)
		{
			memory_pointer[0] = c;
		}
		if ((value & 0x40) != 0)
		{
			memory_pointer[1] = c;
		}
		if ((value & 0x20) != 0)
		{
			memory_pointer[2] = c;
		}
		if ((value & 0x10) != 0)
		{
			memory_pointer[3] = c;
		}
		if ((value & 0x08) != 0)
		{
			memory_pointer[4] = c;
		}
		if ((value & 0x04) != 0)
		{
			memory_pointer[5] = c;
		}
		if ((value & 0x02) != 0)
		{
			memory_pointer[6] = c;
		}
		if ((value & 0x01) != 0)
		{
			memory_pointer[7] = c;
		}
	}
	return;
}

void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484, 0, 0, x - 1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6, 0, y - 28, x - 1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF, 0, y - 27, x - 1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6, 0, y - 26, x - 1, y - 1);

	boxfill8(vram, x, COL8_FFFFFF, 3, y - 24, 59, y - 24);
	boxfill8(vram, x, COL8_FFFFFF, 2, y - 24, 2, y - 4);
	boxfill8(vram, x, COL8_848484, 3, y - 4, 59, y - 4);
	boxfill8(vram, x, COL8_848484, 59, y - 23, 59, y - 5);
	boxfill8(vram, x, COL8_000000, 2, y - 3, 59, y - 3);
	boxfill8(vram, x, COL8_000000, 60, y - 24, 60, y - 3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x - 4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y - 4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y - 3, x - 4, y - 3);
	boxfill8(vram, x, COL8_FFFFFF, x - 3, y - 24, x - 3, y - 3);
	return;
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {// rgb, like (255,0,255) is one color, red, blue, green
											  0x00, 0x00, 0x00,
											  0xff, 0x00, 0x00,
											  0x00, 0xff, 0x00,
											  0xff, 0xff, 0x00,
											  0x00, 0x00, 0xff,
											  0xff, 0x00, 0xff,
											  0x00, 0xff, 0xff,
											  0xff, 0xff, 0xff,
											  0xc6, 0xc6, 0xc6,
											  0x84, 0x00, 0x00,
											  0x00, 0x84, 0x00,
											  0x84, 0x84, 0x00,
											  0x00, 0x00, 0x84,
											  0x84, 0x00, 0x84,
											  0x00, 0x84, 0x84,
											  0x84, 0x84, 0x84};
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags(); /* store Where you can io interrupt, exactly EFLAGS register*/
	io_cli();				   /* disable interrupt */
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags); /*by the way recover io interrupt */
	return;
}

// xsize 参数在 boxfill8 函数中用于计算每个像素在 VRAM 中的偏移量，从而能够正确地访问和修改像素数据。它是实现矩形填充功能的关键参数之一。
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++)
	{
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

//---
void init_gdtidt(void)
{//它将 GDT 和 IDT 的所有描述符初始化为 0，
//然后设置了几个特定的描述符，并将 GDT 和 IDT 的地址和界限加载到 GDTR 和 IDTR 寄存器中。
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;
	int i;

	/* GDT initial */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	/* IDT initial */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{//ar = Access Rights Byte（访问权限字节），它是段描述符或门描述符中的一个重要组成部分，用于控制对该段或门的访问权限。
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
