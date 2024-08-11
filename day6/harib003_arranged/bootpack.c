/* to tell C compiler there are functions built with other files*/
#include <stdio.h>
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);


struct BOOTINFO
{
	char cyls, leds, vmode, reserve; // every occupy 1 byte
	short scrnx, scrny;				 // every occupy 2 bytes
	char *vram;						 // 4bytes
};
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

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
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_ascii_str(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
void init_mouse_cursor8(char *mouse, char bc);

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
