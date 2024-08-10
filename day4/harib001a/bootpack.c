/* to tell C compiler there are functions built with other files*/

void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{

//fin:
//	io_hlt(); 
//	goto fin;
	int i; //32 bit
	
	for(i = 0xa0000 ; i<=0xaffff ; i++){  // 8*320*200 = 16*32*1000 ~= 2^19(?)bits = 2^16(?)bytes
		write_mem8(i, 15); //MOV BYTE [i],15  //15 means white color here
	}
	
	for(;;){
		io_hlt();
	}


}
