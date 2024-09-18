#include "bootpack.h"

void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* disable all interrups , PIC0 */
	io_out8(PIC1_IMR,  0xff  ); /* disable all interrups , PIC1 */

	io_out8(PIC0_ICW1, 0x11  ); /* （edge trigger mode） */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 will be received by AINT20-27 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 is connected by IRQ2 to PIC0 */
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* （edge trigger mode） */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 will be received by AINT28-2f */
	io_out8(PIC1_ICW3, 2     ); /* PIC1 is connected by IRQ2 */
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 disable all interrups, except PIC1*/
	io_out8(PIC1_IMR,  0xff  ); /* 11111111  disable all interrups */

	return;
}

void inthandler27(int *esp)
//该中断由PIC初始化时产生
{
	io_out8(PIC0_OCW2, 0x67); /* 向PIC通知IRQ-07受理完成*/
	return;
}
