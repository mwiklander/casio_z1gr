/*
	CASIO Z-1/FX-890P emulator
*/

#ifndef Z1_H
#define Z1_H

#include <stdio.h>
#include <limits.h>
#include "i80x86.h"
#include "conf.h"

/* 真偽 */
#define FALSE	0	/* 偽 */
#define TRUE	1	/* 真 */

/* 機種 */
#define MACHINE_Z1	0	/* Z-1 */
#define MACHINE_Z1GR	1	/* Z-1GR */
#define MACHINE_Z1GRA	2	/* Z-1GRa */
#define MACHINE_FX890P	3	/* FX-890P */
#define MACHINE_FX890P_EN	4	/* FX-890P海外モデル */

/* LCD */
#define VRAM_HEIGHT	65	/* VRAM 高さ */
#define VRAM_WIDTH	32	/* VRAM 幅バイト数 */
#define LCD_HEIGHT	32	/* LCD 高さ */
#define LCD_WIDTH	192	/* LCD 幅ドット数 */

/* キーコード */
#define ZKEY_BRK	0x01	/* BRK/ONキー */
#define ZKEY_TAB	0x12	/* TABキー */
#define ZKEY_Q	0x13	/* Qキー */
#define ZKEY_ALLRESET	0x14	/* オールリセットボタン */
#define ZKEY_A	0x15	/* Aキー */
#define ZKEY_CAPS	0x16	/* CAPS/カナキー */
#define ZKEY_Z	0x17	/* Zキー */
#define ZKEY_W	0x22	/* Wキー */
#define ZKEY_E	0x23	/* Eキー */
#define ZKEY_S	0x24	/* Sキー */
#define ZKEY_D	0x25	/* Dキー */
#define ZKEY_X	0x26	/* Xキー */
#define ZKEY_C	0x27	/* Xキー */
#define ZKEY_SRCH	0x28	/* SRCHキー */
#define ZKEY_IN	0x29	/* INキー */
#define ZKEY_R	0x32	/* Rキー */
#define ZKEY_T	0x33	/* Tキー */
#define ZKEY_F	0x34	/* Fキー */
#define ZKEY_G	0x35	/* Gキー */
#define ZKEY_V	0x36	/* Vキー */
#define ZKEY_B	0x37	/* Bキー */
#define ZKEY_OUT	0x38	/* OUTキー */
#define ZKEY_CALC	0x39	/* CALCキー */
#define ZKEY_Y	0x42	/* Yキー */
#define ZKEY_U	0x43	/* Uキー */
#define ZKEY_H	0x44	/* Hキー */
#define ZKEY_J	0x45	/* Jキー */
#define ZKEY_N	0x46	/* Nキー */
#define ZKEY_M	0x47	/* Mキー */
#define ZKEY_SPC	0x48	/* スペースキー */
#define ZKEY_EQUAL	0x49	/* =キー */
#define ZKEY_I	0x52	/* Iキー */
#define ZKEY_O	0x53	/* Oキー */
#define ZKEY_K	0x54	/* Kキー */
#define ZKEY_L	0x55	/* Lキー */
#define ZKEY_COMMA	0x56	/* ,キー */
#define ZKEY_INS	0x57	/* INSキー */
#define ZKEY_LEFT	0x58	/* ←キー */
#define ZKEY_DOWN	0x59	/* ↓キー */
#define ZKEY_P	0x62	/* Pキー */
#define ZKEY_2ND	0x63	/* [S]キー */
#define ZKEY_SEMICOLON	0x64	/* ;キー */
#define ZKEY_COLON	0x65	/* :キー */
#define ZKEY_UP	0x66	/* 上キー */
#define ZKEY_DEL	0x67	/* DELキー */
#define ZKEY_RIGHT	0x68	/* →キー */
#define ZKEY_0	0x69	/* 0キー */
#define ZKEY_MENU	0x72	/* MENUキー */
#define ZKEY_LOG	0x73	/* LOGキー */
#define ZKEY_MR	0x74	/* MRキー */
#define ZKEY_7	0x75	/* 7キー */
#define ZKEY_4	0x76	/* 4キー */
#define ZKEY_1	0x77	/* 1キー */
#define ZKEY_PERIOD	0x78	/* .キー */
#define ZKEY_EXP	0x79	/* Eキー */
#define ZKEY_RETURN	0x7a	/* リターンキー */
#define ZKEY_CAL	0x82	/* CALキー */
#define ZKEY_LN	0x83	/* lnキー */
#define ZKEY_MPLUS	0x84	/* M+キー */
#define ZKEY_8	0x85	/* 8キー */
#define ZKEY_5	0x86	/* 5キー */
#define ZKEY_2	0x87	/* 2キー */
#define ZKEY_3	0x88	/* 3キー */
#define ZKEY_PLUS	0x89	/* +キー */
#define ZKEY_MINUS	0x8a	/* -キー */
#define ZKEY_DEGR	0x92	/* DEGRキー */
#define ZKEY_SIN	0x93	/* sinキー */
#define ZKEY_LPAREN	0x94	/* (キー */
#define ZKEY_RPAREN	0x95	/* )キー */
#define ZKEY_9	0x96	/* 9キー */
#define ZKEY_6	0x97	/* 6キー */
#define ZKEY_ASTER	0x98	/* *キー */
#define ZKEY_SLASH	0x99	/* /キー */
#define ZKEY_BS	0x9a	/* BSキー */
#define ZKEY_SQR	0xa2	/* √キー */
#define ZKEY_X2	0xa3	/* x^2キー */
#define ZKEY_ENG	0xa4	/* ENGキー */
#define ZKEY_CLS	0xa5	/* CLSキー */
#define ZKEY_COS	0xa6	/* cosキー */
#define ZKEY_HAT	0xa7	/* ^キー */
#define ZKEY_ANS	0xa8	/* ANSキー */
#define ZKEY_TAN	0xaa	/* tanキー */
#define ZKEY_SHIFT	0xbc	/* SHIFTキー */
#define ZKEY_COPY	0xfa	/* コピー仮想キー */
#define ZKEY_PASTE	0xfb	/* 貼付仮想キー */
#define ZKEY_REWIND_INPORT	0xfc	/* 入力巻き戻し仮想キー */
#define ZKEY_REWIND_OUTPORT	0xfd	/* 出力巻き戻し仮想キー */
#define ZKEY_DEBUG	0xfe	/* デバッグ仮想キー */
#define ZKEY_OFF	0xff	/* 電源OFF仮想キー */

/* コンビネーションキー */
#define ZKEYMOD_NOSHIFT	0x4000	/* SHIFTなし */
#define ZKEYMOD_SHIFT	0x8000	/* SHIFTあり */
#define ZKEYMOD_MASK	0xc000	/* コンビネーションキーマスク */

/* サウンドレート(Hz) */
#define AUDIO_RATE	44100

/* 大きいほうを戻す */
#define MAX(x, y)	((x) > (y) ? (x): (y))

/* 小さいほうを戻す */
#define MIN(x, y)	((x) < (y) ? (x): (y))

/* 上位・下位バイト */
#if defined(I86_LITTLEENDIAN)
#	define LOW(x)	((uint8 *)&(x) + 0)
#	define HIGH(x)	((uint8 *)&(x) + 1)
#else
#	define LOW(x)	((uint8 *)&(x) + 1)
#	define HIGH(x)	((uint8 *)&(x) + 0)
#endif

/* 64ビット符号なし */
typedef unsigned long long	uint64;
/* 64ビット符号あり */
typedef long long	int64;

/*
	タイマ
*/
struct Timer {
		uint16 interval_a; /* 間隔A */
		uint16 interval_b; /* 間隔B */
		uint16 control; /* 割込 */
		uint16 count; /* カウンタ */
};

/*
	入出力ポート
*/
struct IOPort {
	char path[PATH_MAX]; /* 入出力ファイルのパス名 */
	int pos; /* 読込位置 */
};

/*
	Z-1/FX-890Pの状態
*/
typedef struct {
	I86stat cpu; /* CPU */
	uint8 memory[0x40000 + 0x20000]; /* メモリ */

	uint16 power; /* 電源 */
	uint8 buzzer; /* ブザー */

	/* タイマ */
	struct {
		struct Timer t0; /* タイマ0 */
		struct Timer t1; /* タイマ1 */
		struct Timer t2; /* タイマ2 */
		uint16 control; /* 割込コントロール */
		int intr; /* 割込中か? */
	} timer;

	/* シリアルポート */
	struct {
		uint16 baud; /* ボーレート */
		uint16 settings; /* 設定 */
		uint16 control; /* 割込コントロール */
		int intr; /* 割込中か? */
		int sent; /* 送信済か? */
	} sio;

	/* カードエッジ */
	struct {
		uint16 control; /* 割込コントロール */
		int intr; /* 割込中か? */
	} card;

	/* キー */
	struct {
		uint16 strobe; /* ストローブ */
		uint16 matrix[12]; /* マトリクス */
		uint16 key_control; /* キー割込コントロール */
		int key_intr; /* キー割込中か? */
		uint16 control; /* 割込コントロール */
		int intr; /* 割込中か? */
	} key;

	/* 電源スイッチ */
	struct {
		uint16 control; /* 割込コントロール */
		int intr; /* 割込中か? */
	} sw;

	/* FDD */
	struct {
		uint8 cmd; /* コマンド */
		uint8 param[32], *param_p; /* パラメータ */
		uint8 err; /* エラー */
	} fdd;

	/* VRAM */
	struct {
		uint8 ar; /* 表示メモリ */
		uint8 xar; /* Xアドレスレジスタ */
		uint8 yar; /* Yアドレスレジスタ */
		uint8 fcr; /* コントロールレジスタ */
		uint8 mdr; /* モードレジスタ */
		uint8 csr; /* Cセレクトレジスタ */
		uint8 vram[(LCD_WIDTH / 8) * LCD_HEIGHT]; /* LCD */
		uint8 status; /* シンボルガイド(01:CAPS, 04:S, 08:BASIC, 10:DEG, 20:RAD, 80:GRA) */
	} vram;

	/* 演算 */
	struct {
		uint8 op; /* 演算 */
		uint8 map; /* 演算結果をマップするか? */
		uint8 reg[9]; /* 内部レジスタ */
	} cal;

	/* LCD */
	struct Lcd {
		int pix[LCD_HEIGHT * LCD_WIDTH]; /* ドットのピクセルの濃度 */
		int symbol[6]; /* シンボルの濃度 */
	} lcd;

	/* シリアル受信データ */
	struct IOPort rs_receive;

	/* シリアル送信データ */
	struct IOPort rs_send;

	/* プリンタ出力データ */
	struct IOPort printer;

	/* フロッピーディスク */
	struct Disk {
		char dir[PATH_MAX]; /* 仮想フロッピーディスクのディレクトリ */
		FILE *fp; /* オープンしたファイルのディスクリプタ */
		size_t size; /* オープンしたファイルのサイズ */
		off_t pos; /* オープンしたファイルの読込/書込位置 */
		int result_count; /* 結果読込カウンタ */
		uint16 len; /* バッファデータ長 */
		uint8 *files; /* ファイル検索結果 */
		uint8 *files_p; /* ファイル検索結果の読取ポインタ */
		uint8 data; /* 読込データ */
	} disk;

	/* サウンド */
	struct Sound {
		int8 *buffer[2]; /* サウンドバッファ */
		int len; /* バッファ長 */
		int page; /* 再生中のページ */
		int last_states; /* 最後に出力した時点の経過ステート数 */
		int8 last_vol; /* 最後に書き込んだボリューム */
		int last_pos; /* 最後に書き込んだ位置 */
		int played; /* 再生したか? */
	} sound;

	/* ジョイスティック */
	struct Joystick {
		uint8 right;	/* 右に割り当てられたキー */
		uint8 left;	/* 左に割り当てられたキー */
		uint8 up;	/* 上に割り当てられたキー */
		uint8 down;	/* 下に割り当てられたキー */
		uint8 button[16];	/* ボタンに割り当てられたキー */
	} joy;

	/* 自動入力 */
	struct AutoKey {
		uint8 *key; /* 入力中のキー */
		uint8 press; /* 押しているか? */
		int count; /* 待ちカウンタ */
	} auto_key;

	/* 設定 */
	struct Setting {
		int machine; /* 機種 */
		const char *machine_name; /* 機種名 */
		int cpu_clock; /* CPUクロック数(Hz) */
		uint16 iram_size;	/* 内蔵RAM容量 */
		uint16 oram_size;	/* 拡張RAM容量 */
		int refresh_rate; /* I/O更新レート(Hz) */
		int zoom; /* LCD倍率 */
		int scales;	/* LCD階調数 */
		char path_rom[PATH_MAX]; /* ROMイメージファイルのパス名 */
		char path_ram[PATH_MAX]; /* RAMイメージファイルのパス名 */
	} setting;
} Z1stat;

/* init.c */
int getAudioSamples(int *fps);
int init(Z1stat *, int, char **);

/* depend.c */
void showError(const char *, ...);
void updateWindow(Z1stat *);
int updateKey(Z1stat *);
void delay(int);
void setTitle(const char *);
int initDepend(Z1stat *);

/* z1.c */
char *getDirName(char *, const char *);
int getTimerCount(Z1stat *, const struct Timer *);
void setTimerCount(Z1stat *, struct Timer *, int);
int sendIOData(struct IOPort *, uint8);
int peekIOData(struct IOPort *, uint8 *);
int receiveIOData(struct IOPort *, uint8 *);
void setIOData(struct IOPort *, const char *);
int64 decodeMan(const uint8 *);
uint8 *encodeMan(uint8 *, int64);
void opXY(uint8, const uint8 *, const uint8 *, uint8 *, uint8 *);
char *decodeReg(char *, const uint8 *);
char *decodeValue(char *, const uint8 *);
void writeSound(Z1stat *, uint8);
void flipSoundBuffer(Z1stat *);
uint8 getFdFreeSize(struct Disk *, uint32 *);
uint8 openFdFile(struct Disk *, const uint8 *, const uint8 *);
uint8 closeFdFile(struct Disk *);
uint8 readFdFile(struct Disk *, uint8 *);
uint8 writeFdFile(struct Disk *, uint8, const uint8 *);
uint8 renameFdFile(struct Disk *, const uint8 *, const uint8 *);
uint8 deleteFdFile(struct Disk *, const uint8 *);
uint8 findFdFile(struct Disk *, const uint8 *, uint8 *);
uint8 getFoundFdFile(struct Disk *, uint8 *);
uint8 formatFd(struct Disk *);
int autoUpdateKey(Z1stat *);
void setAutoKey(Z1stat *, uint8);
void setAutoText(Z1stat *, const char *);
int pressKey(Z1stat *, uint8);
int releaseKey(Z1stat *, uint8);

#if defined(DEF_GLOBAL)
#	define EXTERN
#else
#	define EXTERN	extern
#endif

#include "depend.h"

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
