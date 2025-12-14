/*
	CASIO Z1/FX-890P emulator
	メモリ
*/

#include <stdio.h>
#include "z1.h"

#define BUSY	14/*10*/

/*
	VRAMに点を書き込む (writeVramの下請け)
*/
static inline uint8 point(uint8 *vram, int x, int y)
{
	if(x < 0 || x >= LCD_WIDTH)
		return 0;
	else
		return vram[y * (LCD_WIDTH / 8) + (x / 8)];
}

/*
	VRAMを読み込む (i86read8の下請け)
*/
static inline uint8 readVram(I86stat *cpu, int p)
{
	Z1stat *z1 = cpu->i.user_data;
	int x, y;

	if(p & 1)
		switch(z1->vram.ar & 7) {
		case 0: /* 表示メモリ */
			cpu->i.op_states += 8 * BUSY; /* ??? */

			if(z1->vram.fcr & 0x40) {
				if(z1->vram.xar == 5) {
					y = (z1->vram.yar >> 2) & 0x07;
					return z1->vram.status & (0x01 << y);
				} else if(6 <= z1->vram.xar && z1->vram.xar < VRAM_WIDTH && z1->vram.yar < VRAM_HEIGHT) {
					x = (z1->vram.yar / 32) * 96 + (z1->vram.xar - 6) * 6;
					y = (z1->vram.yar % 32);
					return
					(point(z1->vram.vram, x + 0, y) ? 0x20: 0) |
					(point(z1->vram.vram, x + 1, y) ? 0x10: 0) |
					(point(z1->vram.vram, x + 2, y) ? 0x08: 0) |
					(point(z1->vram.vram, x + 3, y) ? 0x04: 0) |
					(point(z1->vram.vram, x + 4, y) ? 0x02: 0) |
					(point(z1->vram.vram, x + 5, y) ? 0x01: 0);
				}
			} else {
				if(z1->vram.xar == 3) {
					y = (z1->vram.yar >> 2) & 0x07;
					return z1->vram.status & (0x01 << y);
				} else if(4 <= z1->vram.xar && z1->vram.xar < VRAM_WIDTH && z1->vram.yar < VRAM_HEIGHT) {
					x = (z1->vram.yar / 32) * 96 + (z1->vram.xar - 4) * 8;
					y = (z1->vram.yar % 32);
					return
					(point(z1->vram.vram, x - 4, y) ? 0x80: 0) |
					(point(z1->vram.vram, x - 3, y) ? 0x40: 0) |
					(point(z1->vram.vram, x - 2, y) ? 0x20: 0) |
					(point(z1->vram.vram, x - 1, y) ? 0x10: 0) |
					(point(z1->vram.vram, x + 0, y) ? 0x08: 0) |
					(point(z1->vram.vram, x + 1, y) ? 0x04: 0) |
					(point(z1->vram.vram, x + 2, y) ? 0x02: 0) |
					(point(z1->vram.vram, x + 3, y) ? 0x01: 0);
				}
			}
		case 1: /* Xアドレスレジスタ */
			return z1->vram.xar;
		case 2: /* Yアドレスレジスタ */
			return z1->vram.yar;
		case 3: /* コントロールレジスタ */
			return z1->vram.fcr;
		case 4: /* モードレジスタ */
			return z1->vram.mdr;
		case 5: /* Cセレクトレジスタ */
			return z1->vram.csr;
		default: /* 無効 */
			return 0;
		}
	else
		return z1->vram.ar;
}

/*
	VRAMに点を書き込む (writeVramの下請け)
*/
static inline void pset(uint8 *vram, int x, int y, int pix)
{
	if(x < 0 || x >= LCD_WIDTH)
		return;

	if(pix)
		vram[y * (LCD_WIDTH / 8) + (x / 8)] |= (0x80 >> (x % 8));
	else
		vram[y * (LCD_WIDTH / 8) + (x / 8)] &= ~(0x80 >> (x % 8));
}

/*
	VRAMに書き込む (i86write8の下請け)
*/
static inline void writeVram(I86stat *cpu, int p, uint8 v)
{
	Z1stat *z1 = cpu->i.user_data;
	int x, y;

	if(p & 1) {
		switch(z1->vram.ar & 7) {
		case 0: /*表示メモリ */
			cpu->i.op_states += 8 * BUSY; /* ??? */

			if(z1->vram.fcr & 0x40) {
				if(z1->vram.xar == 5) {
					y = (z1->vram.yar >> 2) & 0x07;
					if(v)
						z1->vram.status |= (0x01 << y);
					else
						z1->vram.status &= ~(0x01 << y);
				} else if(6 <= z1->vram.xar && z1->vram.xar < VRAM_WIDTH && z1->vram.yar < VRAM_HEIGHT) {
					x = (z1->vram.yar / 32) * 96 + (z1->vram.xar - 6) * 6;
					y = (z1->vram.yar % 32);
					pset(z1->vram.vram, x + 0, y, v & 0x20);
					pset(z1->vram.vram, x + 1, y, v & 0x10);
					pset(z1->vram.vram, x + 2, y, v & 0x08);
					pset(z1->vram.vram, x + 3, y, v & 0x04);
					pset(z1->vram.vram, x + 4, y, v & 0x02);
					pset(z1->vram.vram, x + 5, y, v & 0x01);
				}
			} else {
				if(z1->vram.xar == 3) {
					y = (z1->vram.yar >> 2) & 0x07;
					if(v)
						z1->vram.status |= (0x01 << y);
					else
						z1->vram.status &= ~(0x01 << y);
				} else if(4 <= z1->vram.xar && z1->vram.xar < VRAM_WIDTH && z1->vram.yar < VRAM_HEIGHT) {
					x = (z1->vram.yar / 32) * 96 + (z1->vram.xar - 4) * 8;
					y = (z1->vram.yar % 32);
					pset(z1->vram.vram, x - 4, y, v & 0x80);
					pset(z1->vram.vram, x - 3, y, v & 0x40);
					pset(z1->vram.vram, x - 2, y, v & 0x20);
					pset(z1->vram.vram, x - 1, y, v & 0x10);
					pset(z1->vram.vram, x + 0, y, v & 0x08);
					pset(z1->vram.vram, x + 1, y, v & 0x04);
					pset(z1->vram.vram, x + 2, y, v & 0x02);
					pset(z1->vram.vram, x + 3, y, v & 0x01);
				}
			}
			if(z1->vram.fcr & 0x80)
				z1->vram.xar++;
			else
				z1->vram.yar++;
			break;
		case 1: /* Xアドレスレジスタ */
			cpu->i.op_states += BUSY + BUSY / 2; /* ??? */
			z1->vram.xar = v;
			break;
		case 2: /* Yアドレスレジスタ */
			cpu->i.op_states += BUSY + BUSY / 2; /* ??? */
			z1->vram.yar = v;
			break;
		case 3: /* コントロールレジスタ */
			z1->vram.fcr = v;
			break;
		case 4: /* モードレジスタ */
			z1->vram.mdr = v & 0x1f;
			break;
		case 5: /* Cセレクトレジスタ */
			z1->vram.csr = v & 0x3f;
			break;
		}
	} else
		z1->vram.ar = v;
}

/*
	メモリを読み込む (8bit)
*/
uint8 i86read8(I86stat *cpu, uint16 seg, uint16 off)
{
	int p = (((int )seg << 4) + off) & 0xfffff;

	if(p < 0x40000) {
		if(p >= 0x0420) /* RAM */
			return cpu->m[p];
		else if(p >= 0x0419) /* メモリなし */
			return p & 0xff;
		else if(p >= 0x0410) /* Y */
			return cpu->m[p];
		else if(p >= 0x0409) /* メモリなし */
			return p & 0xff;
		else if(p >= 0x0400) {
			Z1stat *z1 = cpu->i.user_data;

			if(z1->cal.map) { /* 演算結果 */
				uint8 a[16], f;

				opXY(z1->cal.op, &cpu->m[0x0400], &cpu->m[0x0410], a, &f);
				return a[p - 0x0400];
			} else /* X */
				return cpu->m[p];
		} else /* RAM */
			return cpu->m[p];
	} else if(p < 0xa0000) /* メモリなし */
		return p & 0xff;
	else if(p < 0xb0000) /* VRAM */
		return readVram(cpu, p);
	else if(p < 0xe0000) /* ROM */
		return p & 0xff;
	else /* ROM */
		return cpu->m[p - 0xe0000 + 0x40000];
}

/*
	メモリに書き込む (8bit)
*/
void i86write8(I86stat *cpu, uint16 seg, uint16 off, uint8 v)
{
	int p = (((int )seg << 4) + off) & 0xfffff;

	if(p < 0x40000) /* RAM */
		cpu->m[p] = v;
	else if(p < 0xa0000) /* ROMまたはメモリなし */
		return;
	else if(p < 0xb0000) /* VRAM */
		writeVram(cpu, p, v);
}

/*
	メモリを読み込む (16bit)
*/
uint16 i86read16(I86stat *cpu, uint16 seg, uint16 off)
{
	return ((uint16 )i86read8(cpu, seg, off + 1) << 8) + (uint16 )i86read8(cpu, seg, off);
}

/*
	メモリに書き込む (16bit)
*/
void i86write16(I86stat *cpu, uint16 seg, uint16 off, uint16 v)
{
	i86write8(cpu, seg, off + 0, v & 0xff);
	i86write8(cpu, seg, off + 1, v >> 8);
}

/*
	Copyright 2009~2019 maruhiro
	All rights reserved. 

	Redistribution and use in source and binary forms, 
	with or without modification, are permitted provided that 
	the following conditions are met: 

	 1. Redistributions of source code must retain the above copyright notice, 
	    this list of conditions and the following disclaimer. 

	 2. Redistributions in binary form must reproduce the above copyright notice, 
	    this list of conditions and the following disclaimer in the documentation 
	    and/or other materials provided with the distribution. 

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
	THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
