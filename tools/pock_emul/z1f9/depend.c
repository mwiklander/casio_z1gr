/*
	CASIO Z-1/FX-890P emulator
	環境依存部分
*/

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "z1.h"
#include "caps1.xbm"
#include "s1.xbm"
#include "basic1.xbm"
#include "deg1.xbm"
#include "rad1.xbm"
#include "gra1.xbm"
#include "dot1.xbm"
#include "caps2.xbm"
#include "s2.xbm"
#include "basic2.xbm"
#include "deg2.xbm"
#include "rad2.xbm"
#include "gra2.xbm"
#include "dot2.xbm"
#include "caps3.xbm"
#include "s3.xbm"
#include "basic3.xbm"
#include "deg3.xbm"
#include "rad3.xbm"
#include "gra3.xbm"
#include "dot3.xbm"

#if SDL_MAJOR_VERSION == 1
static const SDL_VideoInfo *video;
#endif
#if SDL_MAJOR_VERSION == 2
static SDL_Window *window;
#endif
static SDL_Surface *screen;
static SDL_Joystick *joy = NULL;
static Uint32 pixLCD[256], pixLCDoff, pixFrame, pixText;

static SDL_Rect rectCaps = { 0, 0, caps1_width, caps1_height }, rectCapsDot = { caps1_width, 0, dot1_width, dot1_height };
static SDL_Rect rectS = { 0, 5, s1_width, s1_height }, rectSDot = { s1_width, 6, dot1_width, dot1_height };
static SDL_Rect rectBasic = { 0, 12, basic1_width, basic1_height }, rectBasicDot = { basic1_width, 12, dot1_width, dot1_height };
static SDL_Rect rectDeg = { 0, 18, deg1_width, deg1_height }, rectDegDot = { deg1_width, 18, dot1_width, dot1_height };
static SDL_Rect rectRad = { 0, 24, rad1_width, rad1_height }, rectRadDot = { rad1_width, 24, dot1_width, dot1_height };
static SDL_Rect rectGra = { 0, 30, gra1_width, gra1_height }, rectGraDot = { gra1_width, 30, dot1_width, dot1_height };
static SDL_Rect rectFrame = { 0, 0, 10, 34 };
static SDL_Rect rectSymbol[] = {
	{ 10, 1, 3, 2 },
	{ 10, 7, 3, 2 },
	{ 10, 13, 3, 2 },
	{ 10, 19, 3, 2 },
	{ 10, 25, 3, 2 },
	{ 10, 31, 3, 2 }
};
static SDL_Rect rectPanel = { 10, 0, 196, 34 };
static SDL_Rect rectLCD = { 14, 1, LCD_WIDTH, LCD_HEIGHT };

static unsigned char *caps_bits[] = { caps1_bits, caps2_bits, caps3_bits };
static unsigned char *s_bits[] = { s1_bits, s2_bits, s3_bits };
static unsigned char *basic_bits[] = { basic1_bits, basic2_bits, basic3_bits };
static unsigned char *deg_bits[] = { deg1_bits, deg2_bits, deg3_bits };
static unsigned char *rad_bits[] = { rad1_bits, rad2_bits, rad3_bits };
static unsigned char *gra_bits[] = { gra1_bits, gra2_bits, gra3_bits };
static unsigned char *dot_bits[] = { dot1_bits, dot2_bits, dot3_bits };

static const char *machineName;
static int zoom;

/*
	エラーを表示する
*/
void showError(const char *format, ...)
{
	va_list v;
#if SDL_MAJOR_VERSION == 2
	char buf[256];
#endif

	va_start(v, format);
#if SDL_MAJOR_VERSION == 2
	vsprintf(buf, format, v);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", buf, NULL);
#elif SDL_MAJOR_VERSION == 1
	vfprintf(stderr, format, v);
#endif
	va_end(v);
}

/*
	点を描く(下請け)
*/
inline static void pset(int x, int y, Uint32 *pix)
{
	Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + screen->format->BytesPerPixel * x;

	switch(screen->format->BytesPerPixel) {
	case 1:
		*p = (Uint8 )*pix;
		break;
	case 2:
		*(Uint16 *)p = (Uint16 )*pix;
		break;
	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (*pix >> 16) & 0xff;
			p[1] = (*pix >> 8) & 0xff;
			p[2] = *pix & 0xff;
		} else {
			p[0] = *pix & 0xff;
			p[1] = (*pix >> 8) & 0xff;
			p[2] = (*pix >> 16) & 0xff;
		}
		break;
	case 4:
		*(Uint32 *)p = *pix;
		break;
	}
}

/*
	塗りつぶした四角を描く
*/
inline static void boxfill(SDL_Rect *r, Uint32 *pix)
{
	int x, y;

	for(y = r->y; y < r->y + r->h; y++)
		for(x = r->x; x < r->x + r->w; x++)
			pset(x, y, pix);
}

/*
	LCDに点を描く
*/
inline static void putLCD(int x, int y, int scale)
{
	Uint32 *pix = &pixLCD[scale];
	int i, j;

	for(j = rectLCD.y + y * zoom; j < rectLCD.y + (y + 1) * zoom; j++)
		for(i = rectLCD.x + x * zoom; i < rectLCD.x + (x + 1) * zoom; i++)
			pset(i, j, pix);
}

/*
	ガイドシンボルを描く
*/
inline static void putSymbol(int y, int scale)
{
	boxfill(&rectSymbol[y], &pixLCD[scale]);
}

/*
	ビットマップを表示する
*/
static void putBitmap(SDL_Rect *r, const unsigned char *bits, Uint32 *pix_off, Uint32 *pix_on)
{
	int i, j;
	const unsigned char *p = bits;

	for(j = r->y; j < r->y + r->h; j++)
		for(i = r->x; i < r->x + r->w; i += 8) {
			pset(i + 0, j, *p & 0x01 ? pix_on: pix_off);
			pset(i + 1, j, *p & 0x02 ? pix_on: pix_off);
			pset(i + 2, j, *p & 0x04 ? pix_on: pix_off);
			pset(i + 3, j, *p & 0x08 ? pix_on: pix_off);
			pset(i + 4, j, *p & 0x10 ? pix_on: pix_off);
			pset(i + 5, j, *p & 0x20 ? pix_on: pix_off);
			pset(i + 6, j, *p & 0x40 ? pix_on: pix_off);
			pset(i + 7, j, *p & 0x80 ? pix_on: pix_off);
			p++;
		}
}

/*
	領域を拡大する
*/
static SDL_Rect *zoomRect(SDL_Rect *r, int zoom)
{
	r->x *= zoom;
	r->y *= zoom;
	r->w *= zoom;
	r->h *= zoom;
	return r;
}

/*
	ビットマップの領域を拡大する
*/
static SDL_Rect *zoomBitmap(SDL_Rect *r, int zoom)
{
	if(zoom <= 3)
		return zoomRect(r, zoom);

	r->x = r->x * zoom + (r->w * zoom - r->w * 3) / 2;
	r->y = r->y * zoom + (r->h * zoom - r->h * 3) / 2;
	r->w = r->w * 3;
	r->h = r->h * 3;
	return r;
}

/*
	表示を更新する
*/
void updateWindow(Z1stat *z1)
{
	int lock, x, y;

	/* ウィンドウに描画する */
	if((lock = SDL_MUSTLOCK(screen)))
		if(SDL_LockSurface(screen) < 0)
			return;

	for(y = 0; y < LCD_HEIGHT; y++)
		for(x = 0; x < LCD_WIDTH; x++)
			putLCD(x, y, z1->lcd.pix[y * LCD_WIDTH + x]);
	for(y = 0; y < 6; y++)
		putSymbol(y, z1->lcd.symbol[y]);

	if(lock)
		SDL_UnlockSurface(screen);
#if SDL_MAJOR_VERSION == 2
	SDL_UpdateWindowSurface(window);
#elif SDL_MAJOR_VERSION == 1
	SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif
}

/* 押されたキーのキー変換 */
static uint16 keyConvPressed[KEY_LAST + 1];

/*
	キーの状態を更新する
*/
int updateKey(Z1stat *z1)
{
	SDL_Event e;
#if SDL_MAJOR_VERSION == 2
	SDL_Keymod mod;
#elif SDL_MAJOR_VERSION == 1
	SDLMod mod;
#endif
	int key, intr;
	uint16 zkey;

	if((intr = autoUpdateKey(z1)) > 0)
		return intr;
	if(intr == 0)
		return -1;

	while(SDL_PollEvent(&e)) {
		switch(e.type) {
		case SDL_KEYDOWN: /* キーを押した */
			/* SDLのキーコードを得る */
#if SDL_MAJOR_VERSION == 2
			if(e.key.repeat)
				break;
			key = e.key.keysym.scancode;
			if(key == KEY_CAPSLOCK) {
				setAutoKey(z1, keyConv[key]);
				break;
			}
#elif SDL_MAJOR_VERSION == 1
			key = e.key.keysym.sym;
			if(key == KEY_CAPSLOCK || key == KEY_NUMLOCK || key == KEY_SCROLLOCK) {
				setAutoKey(z1, keyConv[key]);
				break;
			}
#endif
			if(key == 0)
				break;

			/* ポケコンのキーコードを得る */
			mod = SDL_GetModState();
			if((mod & KMOD_CTRL) && keyConvCtrl[key])
				zkey = keyConvCtrl[key];
			else if((mod & KMOD_ALT) && keyConvAlt[key])
				zkey = keyConvAlt[key];
			else if((mod & KMOD_SHIFT) && keyConvShift[key])
				zkey = keyConvShift[key];
			else
				zkey = keyConv[key];
			if((zkey & ~ZKEYMOD_MASK) == 0)
				break;
			/*printf("zkey=%04x\n", zkey);*/

			/* コピーか? */
			if((zkey & ~ZKEYMOD_MASK) == ZKEY_COPY) {
#if SDL_MAJOR_VERSION == 2
				char ans[32];
				SDL_SetClipboardText(decodeValue(ans, &z1->cpu.m[0x1714]));
#endif
				break;
			}
			/* 貼り付けか? */
			if((zkey & ~ZKEYMOD_MASK) == ZKEY_PASTE) {
#if SDL_MAJOR_VERSION == 2
				if(SDL_HasClipboardText())
					setAutoText(z1, SDL_GetClipboardText());
#endif
				break;
			}

			/* キーの押下処理を行う */
			if((intr = pressKey(z1, zkey & ~ZKEYMOD_MASK)) < 0)
				return -1;

			/* コンビネーションキーの処理を行う */
			if(!z1->key.matrix[11] && (zkey & ZKEYMOD_SHIFT)) {
				/*printf("PRESS SHIFT\nPRESS %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
				pressKey(z1, ZKEY_SHIFT);
				keyConvPressed[key] = zkey;
			} else if(z1->key.matrix[11] && (zkey & ZKEYMOD_NOSHIFT)) {
				/*printf("RELEASE SHIFT\nPRESS %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
				releaseKey(z1, ZKEY_SHIFT);
				keyConvPressed[key] = zkey;
			} else {
				/*printf("PRESS %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
				keyConvPressed[key] = zkey & ~ZKEYMOD_MASK;
			}
			return intr;
		case SDL_KEYUP: /* キーを離した */
			/* SDLのキーコードを得る */
#if SDL_MAJOR_VERSION == 2
			if(e.key.repeat)
				break;
			key = e.key.keysym.scancode;
#elif SDL_MAJOR_VERSION == 1
			key = e.key.keysym.sym;
#endif
			if(key == 0)
				break;

			/* ポケコンのキーコードを得る */
			zkey = keyConvPressed[key];
			if((zkey & ~ZKEYMOD_MASK) == 0)
				break;

			/* コピー/貼り付けか? */
			if((zkey & ~ZKEYMOD_MASK) == ZKEY_COPY || (zkey & ~ZKEYMOD_MASK) == ZKEY_PASTE)
				break;

			/* キーの解放処理を行う */
			intr = releaseKey(z1, zkey & ~ZKEYMOD_MASK);

			/* コンビネーションキーの処理を行う */
			mod = SDL_GetModState();
			if(z1->key.matrix[11] && (zkey & ZKEYMOD_SHIFT) && !(mod & KMOD_SHIFT)) {
				/*printf("RELEASE SHIFT\nRELEASE %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
				releaseKey(z1, ZKEY_SHIFT);
			} else if(!z1->key.matrix[11] && (zkey & ZKEYMOD_NOSHIFT) && (mod & KMOD_SHIFT)) {
				/*printf("PRESS SHIFT\nRELEASE %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
				pressKey(z1, ZKEY_SHIFT);
			} else {
				/*printf("RELEASE %02x\n\n", zkey & ~ZKEYMOD_MASK);*/
			}
			keyConvPressed[key] = 0;
			return intr;
		case SDL_MOUSEBUTTONDOWN: /* マウスのボタンをクリックした */
#if SDL_MAJOR_VERSION == 2
			if(e.button.button == SDL_BUTTON_MIDDLE)
				if(SDL_HasClipboardText())
					setAutoText(z1, SDL_GetClipboardText());
#endif
			break;
		case SDL_JOYAXISMOTION: /* ジョイパッドを動かした */
			if(e.jaxis.axis == 0) {
				if(e.jaxis.value < -32768 / 10)
					intr = pressKey(z1, z1->joy.left);
				else
					intr = releaseKey(z1, z1->joy.left);
				if(e.jaxis.value > 32767 / 10)
					intr = pressKey(z1, z1->joy.right);
				else
					intr = releaseKey(z1, z1->joy.right);
			} else if(e.jaxis.axis == 1) {
				if(e.jaxis.value < -32768 / 10)
					intr = pressKey(z1, z1->joy.up);
				else
					intr = releaseKey(z1, z1->joy.up);
				if(e.jaxis.value > 32767 / 10)
					intr = pressKey(z1, z1->joy.down);
				else
					intr = releaseKey(z1, z1->joy.down);
			}
			return intr;
		case SDL_JOYBUTTONDOWN: /* ジョイパッドのボタンを押した */
			if(e.jbutton.button >= sizeof(z1->joy.button) / sizeof(z1->joy.button[0]) || z1->joy.button[e.jbutton.button] == 0)
				break;
			return pressKey(z1, z1->joy.button[e.jbutton.button]);
		case SDL_JOYBUTTONUP: /* ジョイパッドのボタンを離した */
			if(e.jbutton.button >= sizeof(z1->joy.button) / sizeof(z1->joy.button[0]) || z1->joy.button[e.jbutton.button] == 0)
				break;
			return releaseKey(z1, z1->joy.button[e.jbutton.button]);
#if SDL_MAJOR_VERSION == 2
		case SDL_DROPFILE: /* ファイルをドロップした */
			SDL_RaiseWindow(window);
			getDirName(z1->disk.dir, e.drop.file);
			break;
#endif
		case SDL_QUIT: /* 終了した */
			exit(0);
			break;
		}
	}
	return -1;
}

/*
	一定時間待つ
*/
void delay(int interval)
{
	static Uint32 last = 0, left;
	Uint32 now;

	now = SDL_GetTicks();
	if(last + interval > now) {
		left = last + interval - now;
		SDL_Delay(left);
		last = now + left;
	} else
		last = now;
}

/*
	ウィンドウのタイトルを設定する
*/
void setTitle(const char *msg)
{
	char buf[256];

	sprintf(buf, "%s %s\n", machineName, msg);
#if SDL_MAJOR_VERSION == 2
	SDL_SetWindowTitle(window, buf);
#elif SDL_MAJOR_VERSION == 1
	SDL_WM_SetCaption(buf, NULL);
#endif
}

/*
	サウンド再生のコールバック
*/
static SDLCALL void playSound(void *userdata, Uint8 *stream, int len)
{
	struct Sound *sound = &((Z1stat *)userdata)->sound;

#if SDL_MAJOR_VERSION == 2
	memset(stream, 0, len);
#endif
	SDL_MixAudio(stream, (uint8 *)sound->buffer[sound->page], len, SDL_MIX_MAXVOLUME);
	sound->played = TRUE;
}

/*
	LCDの表示濃度を得る
*/
static int pixToScale(int scales, int pix)
{
	if(pix <= 0)
		return 0;
	if(scales <= 0)
		return pix;
	return (pix / (255 / (scales - 1))) * (255 / (scales - 1)) + 1;
}

/*
	環境依存部分を初期化する
*/
int initDepend(Z1stat *z1)
{
	SDL_AudioSpec audio;
	int i, use_joy = z1->joy.right != 0 || z1->joy.left != 0 || z1->joy.up != 0 || z1->joy.down != 0 || z1->joy.button[0] != 0 || z1->joy.button[1] != 0 || z1->joy.button[2] != 0 || z1->joy.button[3] != 0 || z1->joy.button[4] != 0 || z1->joy.button[5] != 0 || z1->joy.button[6] != 0 || z1->joy.button[7] != 0 || z1->joy.button[8] != 0 || z1->joy.button[9] != 0 || z1->joy.button[10] != 0 || z1->joy.button[11] != 0 || z1->joy.button[12] != 0 || z1->joy.button[13] != 0 || z1->joy.button[14] != 0 || z1->joy.button[15] != 0;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | (use_joy ? SDL_INIT_JOYSTICK: 0) | (z1->sound.len > 0 ? SDL_INIT_AUDIO: 0))) {
		fprintf(stderr, "SDL_Init fail. %s\n", SDL_GetError());
		return FALSE;
	}
	atexit(SDL_Quit);

	zoom = z1->setting.zoom;
	zoomRect(&rectFrame, zoom);
	zoomBitmap(&rectCaps, zoom);
	zoomBitmap(&rectCapsDot, zoom);
	zoomBitmap(&rectS, zoom);
	zoomBitmap(&rectSDot, zoom);
	zoomBitmap(&rectBasic, zoom);
	zoomBitmap(&rectBasicDot, zoom);
	zoomBitmap(&rectDeg, zoom);
	zoomBitmap(&rectDegDot, zoom);
	zoomBitmap(&rectRad, zoom);
	zoomBitmap(&rectRadDot, zoom);
	zoomBitmap(&rectGra, zoom);
	zoomBitmap(&rectGraDot, zoom);
	zoomRect(&rectPanel, zoom);
	zoomRect(&rectSymbol[0], zoom);
	zoomRect(&rectSymbol[1], zoom);
	zoomRect(&rectSymbol[2], zoom);
	zoomRect(&rectSymbol[3], zoom);
	zoomRect(&rectSymbol[4], zoom);
	zoomRect(&rectSymbol[5], zoom);
	zoomRect(&rectLCD, zoom);

#if SDL_MAJOR_VERSION == 2
	if((window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, rectPanel.x + rectPanel.w, rectPanel.y + rectPanel.h, 0)) == NULL) {
		fprintf(stderr, "SDL_CreateWindow fail. %s\n", SDL_GetError());
		return FALSE;
	}
	if((screen = SDL_GetWindowSurface(window)) == NULL) {
		fprintf(stderr, "SDL_GetWindowSurface fail. %s", SDL_GetError());
		return FALSE;
	}
#elif SDL_MAJOR_VERSION == 1
	if((video = SDL_GetVideoInfo()) == NULL) {
		fprintf(stderr, "SDL_GetVideoInfo fail. %s\n", SDL_GetError());
		return FALSE;
	}
	if((screen = SDL_SetVideoMode(rectPanel.x + rectPanel.w, rectPanel.y + rectPanel.h, video->vfmt->BitsPerPixel, SDL_HWSURFACE)) == NULL) {
		fprintf(stderr, "SDL_SetVideoMode fail. %s", SDL_GetError());
		return FALSE;
	}
#endif

	machineName = z1->setting.machine_name;
	setTitle("");

	for(i = 0; i < 256; i++) {
		int scale = pixToScale(z1->setting.scales, i);
		pixLCD[i] = SDL_MapRGB(screen->format, 0xaa - (0xaa - 0x00) * scale / 255, 0xdd - (0xdd - 0x22) * scale / 255, 0xbb - (0xbb - 0x11) * scale / 255);
	}
	pixLCDoff = SDL_MapRGB(screen->format, 0xcc, 0xee, 0xbb);
	pixFrame = SDL_MapRGB(screen->format, 0xd4, 0xd0, 0xc8);
	pixText = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);

	boxfill(&rectFrame, &pixFrame);
	putBitmap(&rectCaps, caps_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectCapsDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectS, s_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectSDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectBasic, basic_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectBasicDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectDeg, deg_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectDegDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectRad, rad_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectRadDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectGra, gra_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	putBitmap(&rectGraDot, dot_bits[MIN(zoom, 3) - 1], &pixFrame, &pixText);
	boxfill(&rectPanel, &pixLCDoff);

	if(use_joy)
		if((joy = SDL_JoystickOpen(0)) == NULL)
			;

	if(z1->sound.len > 0) {
		memset(&audio, 0, sizeof(audio));
		audio.freq = AUDIO_RATE;
		audio.format = AUDIO_S8;
		audio.channels = 1;
		audio.samples = z1->sound.len;
		audio.callback = playSound;
		audio.userdata = z1;
		if(SDL_OpenAudio(&audio, NULL) < 0) {
			fprintf(stderr, "SDL_OpenAudio fail. %s", SDL_GetError());
			return 1;
		}
		SDL_PauseAudio(0);
	}
	return TRUE;
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
