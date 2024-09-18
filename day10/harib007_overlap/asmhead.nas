; haribote-os boot asm
; TAB=4
; ?�����V�C?�i��?�I��p����������n???�I�����y?�H��C�?�u�d���A��?�K�v�I���� (bootpack)�C
; ���?���T��?��? bootpack�C�R bootpack ??�@?�I����n?���n���a?�s�B

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

;ORG 0xc200 ?��?���s�\���ڍ�?����??�������?��?���B��??�㐥��???��iassembler�j�C�����I��?�a����??�������n�� 0xc200 ?�n
;��??����� ORG 0xc200 ?��?��?�C����@?�I���L�w�ߘa�����I�n���s?�u?��?�� 0xc200 �I�ΈڗʁB��@

; set the mode of screen(by int)

		MOV		AL,0x13			; specific value for:VGA graphics, 320x200x8bit color
		MOV		AH,0x00
		INT		0x10             ;?�p BIOS ���f 0x10
		MOV		BYTE [VMODE],8	; 8*320*200
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000  ;��? 0x000a0000 ��?�������n�� VRAM ��, �� VGA ?�`�͎����C0x000a0000 �ʏ퐥?���I�N�n�n��

; Get the BIOS to tell you the keyboard  status

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; (�֗p PIC �ڝ��C�����f)
; ���� AT ���e���I?䗁C�@�ʗv���n�� PIC�C�K?��?�s CLI �w�ߔV�O����������C��?�L?��?�v�n?�k�N�B
; PIC �I���n�������c�@?�s�B

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; �����꘢�󑀍�w�� (NOP)�C(��?�ݖ^�����^��C???�s OUT �w�߉\��?�v??�B)
		OUT		0xa1,AL

		CLI						; disable interrupts at the CPU level

; ?�i��?�I��p��?�p A20 �n��?�C��? CPU ?? 1MB �ȏ�I�����B
; A20 Gate ���꘢�T�� A20 �n��?�I???�C�p���T�� CPU ���۔\??? 1MB �ȏ�I�����n���B

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; Switch to protected mode

[INSTRSET "i486p"]				; up to 486 instructions can be used

		LGDT	[GDTR0]			; �� GDTR0 ?�I�S�Ǖ`�q���\ (GDT) ��?�� GDTR �񑶊풆�BGDT ����?�͎����p���`�q�����i�I�\�B
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; �� EAX �񑶊�I�� 31 �ʐ���B?�\���֗p��?�����B
		OR		EAX,0x00000001	; �� EAX �񑶊�I�� 0 ��?�u? 1�B?�\��?�p��?�͎��B
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
; ?�� 8 ��񐏈�??�I�C����\�� GDT�i�S�Ǖ`�q���\�j���I�꘢����i�`�q���I�����C���� 8 ����??���i�`�q����p 8 ����?�B
; ?���i�`�q�� (����? 1) �ʏ��z�u?�꘢ ��?�ʓI�����i�C�󊎛��I��n��? 0�C��?? 4GB�B?�Ӗ�������??���� 32 ��?���n����?�B
		MOV		AX,1*8			;  ��? 8 ��?�� AX �񑶊풆�B?�\��??�꘢��?�ʓI 32 �ʑ�?�i�B
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

; ?����?(data)��?���

		MOV		ESI,0x7c00		; source
		MOV		EDI,DSKCAC		; destination
		MOV		ECX,512/4       ; transfer size
		CALL	memcpy          ; call memcpy function

; the rest

		MOV		ESI,DSKCAC0+512	; �]����
		MOV		EDI,DSKCAC+512	; �]����
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; �V�����_������o�C�g��/4�ɕϊ�
		SUB		ECX,512/4		; IPL�̕�������������
		CALL	memcpy

; asmhead�ł��Ȃ���΂����Ȃ����Ƃ͑S�����I������̂ŁA
;	���Ƃ�bootpack�ɔC����

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
; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

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
