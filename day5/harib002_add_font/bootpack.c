/* to tell C compiler there are functions built with other files*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data); 
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);//+


#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO {
	char cyls, leds, vmode, reserve;// every occupy 1 byte
	short scrnx, scrny;// every occupy 2 bytes
	char *vram;// 4bytes
};

static char font_A[16] = { // character "A": one element means 8 pixels, and '1' bit means fill this block
	0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24, 
	0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00 
}; 
extern char hankaku[4096]; 



void HariMain(void)
{

	char *vram; 
	int xsize, ysize; 
	struct BOOTINFO *binfo;//+

	init_palette(); 

	binfo = (struct BOOTINFO *) 0x0ff0;
	xsize = (*binfo).scrnx;//=base+4
	ysize = (*binfo).scrny;//=base+6
	vram = (*binfo).vram;//=base+4+2*2
	init_screen(vram, xsize, ysize);

	putfont8(vram, xsize, 10, 10, COL8_FFFFFF, font_A);

	putfont8(binfo->vram, binfo->scrnx,  128, 128, COL8_FFFFFF, hankaku + 'A' * 16);
	putfont8(binfo->vram, binfo->scrnx, 16, 8, COL8_FFFFFF, hankaku + 'B' * 16);
	putfont8(binfo->vram, binfo->scrnx, 24, 8, COL8_FFFFFF, hankaku + 'C' * 16);
	putfont8(binfo->vram, binfo->scrnx, 40, 8, COL8_FFFFFF, hankaku + '1' * 16);
	putfont8(binfo->vram, binfo->scrnx, 48, 8, COL8_FFFFFF, hankaku + '2' * 16);
	putfont8(binfo->vram, binfo->scrnx, 56, 8, COL8_FFFFFF, hankaku + '3' * 16);


	for (;;) { 
		io_hlt(); 
	}

}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font) 
{ // char c means "color"
	int i; 
	char *memory_pointer, value; 
	for (i = 0; i < 16; i++) { 
		memory_pointer = vram + (y + i) * xsize + x; 
		value = font[i]; 
		if ((value & 0x80) != 0) { memory_pointer[0] = c; } 
		if ((value & 0x40) != 0) { memory_pointer[1] = c; } 
		if ((value & 0x20) != 0) { memory_pointer[2] = c; } 
		if ((value & 0x10) != 0) { memory_pointer[3] = c; } 
		if ((value & 0x08) != 0) { memory_pointer[4] = c; } 
		if ((value & 0x04) != 0) { memory_pointer[5] = c; } 
		if ((value & 0x02) != 0) { memory_pointer[6] = c; } 
		if ((value & 0x01) != 0) { memory_pointer[7] = c; } 
	} 
	return; 
} 


void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = { //rgb, like (255,0,255) is one color, red, blue, green
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
		0x84, 0x84, 0x84	
	};
	set_palette(0, 15, table_rgb);
	return;

}

void set_palette(int start, int end, unsigned char *rgb) 
{ 
	int i, eflags; 
	eflags = io_load_eflags(); /* store Where you can io interrupt, exactly EFLAGS register*/ 
	io_cli();                /* disable interrupt */ 
	io_out8(0x03c8, start); 
	for (i = start; i <= end; i++) { 
		io_out8(0x03c9, rgb[0] / 4); 
		io_out8(0x03c9, rgb[1] / 4); 
		io_out8(0x03c9, rgb[2] / 4); 
		rgb += 3; 
	} 
	io_store_eflags(eflags); /*by the way recover io interrupt */ 
	return; 
} 

//xsize 参数在 boxfill8 函数中用于计算每个像素在 VRAM 中的偏移量，从而能够正确地访问和修改像素数据。它是实现矩形填充功能的关键参数之一。
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}
