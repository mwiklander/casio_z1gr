/*
	Intel 80186/80188 emulator header
*/

#if !defined(I8086_H)

#include <stdio.h>
#if defined(I86_USE_SDL)
#	include "SDL.h"
#	if SDL_BYTEORDER == SDL_LIL_ENDIAN
#		define I86_LITTLEENDIAN	1
#	else
#		define I86_BIGENDIAN	1
#	endif
#endif

#define I86_RUN	0
#define I86_HALT	1

typedef unsigned char	uint8;
typedef char	int8;
typedef unsigned short	uint16;
typedef short	int16;
typedef unsigned int	uint32;
typedef int	int32;

typedef struct {
	uint8 *m;
	uint16 ax;
	uint16 cx;
	uint16 bx;
	uint16 dx;
	uint16 sp;
	uint16 bp;
	uint16 si;
	uint16 di;
	uint16 cs;
	uint16 ds;
	uint16 es;
	uint16 ss;
	uint16 ip;
	uint16 f;
	uint16 hlt;
	uint16 *prefix;
} I86regs16;

typedef struct {
	uint8 *m;
#if defined(I86_BIGENDIAN)
	uint8 ah, al;
	uint8 ch, cl;
	uint8 bh, bl;
	uint8 dh, dl;
#else 
	uint8 al, ah;
	uint8 cl, ch;
	uint8 bl, bh;
	uint8 dl, dh;
#endif
} I86regs8;

typedef struct {
	I86regs16 pad;
	void *user_data;
	int states;
	int op_states;
	int total_states;
	int trace;
	int bios;
} I86info;

typedef union {
	uint8 *m;
	I86regs16 r16;
	I86regs8 r8;
	I86info i;
} I86stat;

uint8 i86read8(I86stat *, uint16, uint16);
void i86write8(I86stat *, uint16, uint16, uint8);
uint16 i86read16(I86stat *, uint16, uint16);
void i86write16(I86stat *, uint16, uint16, uint16);
uint8 i86inp8(I86stat *, uint16);
uint16 i86inp16(I86stat *, uint16);
int i86out8(I86stat *, uint16, uint8);
int i86out16(I86stat *, uint16, uint16);
void i86reset(I86stat *);
void i86trace(I86stat *);

int i86subroutine(I86stat *, uint32);
int i86vector(I86stat *, int);

int i86disasm(char *, I86stat *, uint16, uint16);
char *i86regs(char *, I86stat *);

int i86nmi(I86stat *);
int i86int(I86stat *, int);
int i86exec(I86stat *);

#endif

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

/* eof */
