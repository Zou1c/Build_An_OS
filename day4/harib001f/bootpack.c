/* to tell C compiler there are functions built with other files*/

void io_hlt(void);
void io_cli(void);//+
void io_out8(int port, int data); //~
int io_load_eflags(void);//+
void io_store_eflags(int eflags);//+

void init_palette(void);//+
void set_palette(int start, int end, unsigned char *rgb);//+

void HariMain(void)
{

//fin:
//	io_hlt(); 
//	goto fin;
	int i; //32 bit
	char* p;

	init_palette();
	
	for(i = 0xa0000 ; i<=0xaffff ; i++){  // 8*320*200 = 16*32*1000 ~= 2^19(?)bits = 2^16(?)bytes
		p=(char*)i;
		*p = i & 0x0f;
		//write_mem8(i, i & 0x0f); //or you can say 0x00000f
	}
	
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