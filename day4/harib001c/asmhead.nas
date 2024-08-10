; haribote-os boot asm
; TAB=4
; ?而言之，?段代?的作用是完成操作系???的早期准?工作，包括?置硬件、加?必要的程序 (bootpack)，
; 并最?将控制?交? bootpack，由 bootpack ??后?的操作系?初始化和?行。

BOTPAK	EQU		0x00280000		; bootpack(load)
DSKCAC	EQU		0x00100000		; disk cache(temp) location
DSKCAC0	EQU		0x00008000		; Disk cache(temp) location [real mode]

; BOOT_INFO
CYLS	EQU		0x0ff0			; which is set by the boot sector
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; video mode
SCRNX	EQU		0x0ff4			; X
SCRNY	EQU		0x0ff6			; Y
VRAM	EQU		0x0ff8			; start address of vedio temp portion

		ORG		0xc200			; where this program is loaded, =0x8000 + 0x4200

;ORG 0xc200 ?条?句并不能直接告?机器??文件被加?到?里。它??上是告???器（assembler），程序的代?和数据??从内存地址 0xc200 ?始
;当??器遇到 ORG 0xc200 ?条?句?，它会将后?的所有指令和数据的地址都?置?相?于 0xc200 的偏移量。例如

; set the mode of screen(by int)

		MOV		AL,0x13			; specific value for:VGA graphics, 320x200x8bit color
		MOV		AH,0x00
		INT		0x10             ;?用 BIOS 中断 0x10
		MOV		BYTE [VMODE],8	; 8*320*200
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000  ;将? 0x000a0000 存?到内存地址 VRAM 中, 在 VGA ?形模式下，0x000a0000 通常是?存的起始地址

; Get the BIOS to tell you the keyboard  status

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; (禁用 PIC 接收任何中断)
; 根据 AT 兼容机的?范，如果要初始化 PIC，必?在?行 CLI 指令之前完成此操作，否?有?会?致系?挂起。
; PIC 的初始化将在稍后?行。

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 插入一个空操作指令 (NOP)，(因?在某些机型上，???行 OUT 指令可能会?致??。)
		OUT		0xa1,AL

		CLI						; disable interrupts at the CPU level

; ?段代?的作用是?用 A20 地址?，允? CPU ?? 1MB 以上的内存。
; A20 Gate 是一个控制 A20 地址?的???，用于控制 CPU 是否能??? 1MB 以上的内存地址。

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; Switch to protected mode

[INSTRSET "i486p"]				; up to 486 instructions can be used

		LGDT	[GDTR0]			; 将 GDTR0 ?的全局描述符表 (GDT) 加?到 GDTR 寄存器中。GDT 是保?模式下用于描述内存段的表。
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 将 EAX 寄存器的第 31 位清零。?表示禁用分?机制。
		OR		EAX,0x00000001	; 将 EAX 寄存器的第 0 位?置? 1。?表示?用保?模式。
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
; ?个 8 并非随意??的，它代表了 GDT（全局描述符表）中的一个特定段描述符的索引，乘以 8 是因??个段描述符占用 8 个字?。
; ?个段描述符 (索引? 1) 通常被配置?一个 可?写的数据段，并且它的基地址? 0，限?? 4GB。?意味着它可以??整个 32 位?性地址空?。
		MOV		AX,1*8			;  将? 8 移?到 AX 寄存器中。?表示??一个可?写的 32 位代?段。
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; copy bootpack to BOTPAK

		MOV		ESI,bootpack	; source address
		MOV		EDI,BOTPAK		; destination address
		MOV		ECX,512*1024/4  ; the size of copy data
		CALL	memcpy          ; call memcpy function

; At the same time, the disk data is transferred to its original location.

; ?制磁?(data)引?扇区

		MOV		ESI,0x7c00		; source
		MOV		EDI,DSKCAC		; destination
		MOV		ECX,512/4       ; transfer size
		CALL	memcpy          ; call memcpy function

; the rest

		MOV		ESI,DSKCAC0+512	; 転送元
		MOV		EDI,DSKCAC+512	; 転送先
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; シリンダ数からバイト数/4に変換
		SUB		ECX,512/4		; IPLの分だけ差し引く
		CALL	memcpy

; asmheadでしなければいけないことは全部し終わったので、
;	あとはbootpackに任せる

; jump to bootpack

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; if no data need to transfer, skip
		MOV		ESI,[EBX+20]	; source
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; destination
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; init of stack pointer
		JMP		DWORD 2*8:0x0000001b ;jump to the address of bootpack

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; if not null, keep wait
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; until the counter to 0
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

		ALIGNB	16
GDT0:
		RESB	8				; Null selector
		DW		0xffff,0x0000,0x9200,0x00cf	; Read/write segment 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; Executable segment 32bit (for bootpack)

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
