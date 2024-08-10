; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; format(obj file)
[BITS 32]						; declare bit-mode of machine
[INSTRSET "i486p"]               ;(+), tell assember the CPU type

; info of 'obj' file

[FILE "naskfunc.nas"]			; source file name

		GLOBAL	_io_hlt,_write_mem8			; function name you write(below)


; function you want to use

[SECTION .text]		; (write this first, and then write program)

_io_hlt:	
; void io_hlt(void);
		HLT
		RET

_write_mem8:    ;将一个字?的数据写入指定的内存地址。(+)
; void write_mem8(int addr, int data);
		MOV 	ECX, [ESP+4]  ; address in [ESP+4]
		MOV 	AL, [ESP+8]   ; data in [ESP+8]
		MOV 	[ECX], AL
		RET