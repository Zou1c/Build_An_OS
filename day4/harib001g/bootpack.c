/* to tell C compiler there are functions built with other files*/

void io_hlt(void);
void io_cli(void);//+
void io_out8(int port, int data); //~
int io_load_eflags(void);//+
void io_store_eflags(int eflags);//+

void init_palette(void);//+
void set_palette(int start, int end, unsigned char *rgb);//+
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

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

void HariMain(void)
{

	char* p;

	init_palette();

	p = (char *) 0xa0000;
	boxfill8(p, 320, COL8_FF0000,  20,  20, 120, 120);
	boxfill8(p, 320, COL8_00FF00,  70,  50, 170, 150);
	boxfill8(p, 320, COL8_0000FF, 120,  80, 220, 180);
	
	for(;;){
		io_hlt();
	}


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
