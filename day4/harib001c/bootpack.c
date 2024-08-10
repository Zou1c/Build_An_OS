/* to tell C compiler there are functions built with other files*/

void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{

//fin:
//	io_hlt(); 
//	goto fin;
	int i; //32 bit
	char* p;
	
	for(i = 0xa0000 ; i<=0xaffff ; i++){  // 8*320*200 = 16*32*1000 ~= 2^19(?)bits = 2^16(?)bytes
		p=i;
		*p = i & 0x0f;
		//write_mem8(i, i & 0x0f); //or you can say 0x00000f
	}
	
	for(;;){
		io_hlt();
	}


}
