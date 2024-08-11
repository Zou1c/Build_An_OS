/* to tell C compiler there are functions built with other files*/
#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{

	char *vram;
	int xsize, ysize;
	struct BOOTINFO *binfo; 
	char mcursor[256];

	binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	xsize = (*binfo).scrnx; //=base+4
	ysize = (*binfo).scrny; //=base+6
	vram = (*binfo).vram;	//=base+4+2*2

	init_gdtidt();
	init_palette();

	init_screen(vram, xsize, ysize);

	
	init_mouse_cursor8(mcursor, COL8_008484); 
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 80, 80, mcursor, 16); 

	//extern char hankaku[4096];
	//putfont8(binfo->vram, binfo->scrnx, 128, 128, COL8_FFFFFF, hankaku + 'A' * 16);

	//putfonts8_ascii_str(vram, xsize, 100, 100, COL8_848484, "hello, string!");

	for (;;)
	{
		io_hlt();
	}
}
