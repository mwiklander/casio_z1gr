/*
	CASIO Z-1/FX-890P emulator
*/

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/statvfs.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#define DEF_GLOBAL
#include "z1.h"
#include "pseudorom.h"

#ifdef _WIN32
/* ファイル操作用一時バッファ (win32専用) */
static wchar_t _wbuffer1[PATH_MAX], _wbuffer2[PATH_MAX];

/*
	UTF-8文字列をwchar_t文字列に変換する(下請け) (win32専用)
*/
static wchar_t *utf8ToWchar(wchar_t *utf16, const char *utf8)
{
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, sizeof(wchar_t) * PATH_MAX);
	return utf16;
}

/*
	ファイルをオープンする(UTF-8対応) (win32専用)
*/
#define fopen(path, mode)	_wfopen(utf8ToWchar(_wbuffer1, path), utf8ToWchar(_wbuffer2, mode))

/*
	ファイルの情報を得る(UTF-8対応) (win32専用)
*/
#define stat	_stat
#define _stat(path, statbuff)	_wstat(utf8ToWchar(_wbuffer1, path), statbuff)

/*
	ファイル名を変更する(UTF-8対応) (win32専用)
*/
#define rename(old_path, new_path)	_wrename(utf8ToWchar(_wbuffer1, old_path), utf8ToWchar(_wbuffer2, new_path))

/*
	ファイル名を削除する(UTF-8対応) (win32専用)
*/
#define unlink(path)	_wunlink(utf8ToWchar(_wbuffer1, path))
#endif

/*
	トレースログを表示する
*/
void i86trace(I86stat *i86)
{
	char buf[256];
	log("%s%dclocks\n\n", i86regs(buf, i86), i86->i.total_states - i86->i.states);
}

int atoix(const char *buf)
{
	int result;

	sscanf(buf, "%x", &result);
	return result;
}

/*
	テキストファイル化されたバイナリファイルを読み込む
*/
static size_t readTextBin(const char *path, size_t buf_size, uint8 *buf)
{
	FILE *fp;
	int i, x;
	uint8 *p = buf;
	char tmp[256];

	if((fp = fopen(path, "r")) == NULL)
		return 0;

	while(!feof(fp) && p < buf + buf_size) {
		fgets(tmp, sizeof(tmp), fp);
		for(i = 0; i < strlen(tmp); i += 2)
			if(isalnum(tmp[i])) {
				sscanf(&tmp[i], "%02x", &x);
				*p++ = (uint8 )x;
			} else
				break;
	}

	fclose(fp);
	return (size_t )(p - buf);
}

/*
	バイナリファイルを読み込む
*/
static size_t readBin(const char *path, size_t buf_size, void *buf)
{
	FILE *fp;
	size_t size;

	if((fp = fopen(path, "rb")) < 0)
		return 0;
	size = fread(buf, 1, buf_size, fp);
	fclose(fp);
	return size;
}

/*
	ROMイメージを読み込む
*/
static size_t readROM(const char *path, uint8 *p)
{
	if(readTextBin(path, 0x20000, p) == 0x20000)
		return TRUE;
	return readBin(path, 0x20000, p) == 0x20000;
}

/*
	ヘッダ付きバイナリファイルを読み込む
*/
static size_t readZ1Bin(const char *path, uint8 *mem, int *ret_top, int *ret_start)
{
	size_t size;
	int top, start;
	uint8 head[16];

	if((size = readBin(path, 16, head)) == 0)
		return 0;
	if(memcmp(head, "\xff\xfe", 2) != 0) {
		top = start = 0x2000;
		size = readBin(path, 0x40000 - top, mem + top);
	} else {
		top = (head[9] << 8U) | head[8];
		if((start = (head[13] << 8U) | head[12]) == 0)
			start = 0x2000;

		if((size = readBin(path, 0x40000 - top - 16, mem + top)) < 16)
			return 0;
		memmove(mem + top, mem + top + 16, size - 16);
		size -= 16;
	}

	if(ret_top != NULL)
		*ret_top = top;
	if(ret_start != NULL)
		*ret_start = start;
	return size;
}

/*
	バイナリファイルを書き込む
*/
static size_t writeBin(const char *path, uint8 *p, size_t size)
{
	FILE *fp;
	size_t written;

	if((fp = fopen(path, "wb")) < 0)
		return 0;
	written = fwrite(p, 1, size, fp);
	fclose(fp);

	return written;
}

/*
	パス名からディレクトリ名を得る
*/
char *getDirName(char *dir, const char *path)
{
	struct stat s;
	const char *p;

	if(stat(path, &s) >= 0 && (s.st_mode & S_IFDIR))
		for(p = path + strlen(path) - 1; p > path && (*p == '/' || *p == '\\'); p--)
			;
	else {
		for(p = path + strlen(path) - 1; p > path && *p != '/' && *p != '\\' && *p != ':'; p--)
			;
		if(p > path)
			p--;
	}

	sprintf(dir, "%-.*s", (int )(p - path + 1), path);
	return dir;
}

/*
	タイマのカウンタを得る
*/
int getTimerCount(Z1stat *z1, const struct Timer *t)
{
	if((t->control & 0x000c) == 0x0008)
		return t->count;
	else
		return t->count - z1->cpu.i.states / 4;
}

/*
	タイマのカウンタを設定する
*/
void setTimerCount(Z1stat *z1, struct Timer *t, int count)
{
	if((t->control & 0x000c) == 0x0008)
		t->count = count;
	else if(z1->cpu.i.states > 0)
		t->count = count + z1->cpu.i.states / 4;
	else
		t->count = count;
}

/*
	タイマの残りカウント数を得る (getExecStatesの下請け)
*/
static int getRestCount(Z1stat *z1, const struct Timer *t)
{
	int max_count = z1->setting.cpu_clock / 800 / 4, interval, rest;

	/* タイマが有効でないか? */
	if(!(t->control & 0x8000))
		return max_count;

	/* タイマ2カウンタ最大でカウンタが進むか? */
	if((t->control & 0x000c) == 0x0008)
		return max_count;

	/* カウンタの最大値を得る */
	if(!(t->control & 0x0002) || (t->control & 0x1000))
		interval = t->interval_a;
	else
		interval = t->interval_b;

	/* 残り時間がI/O更新周期より長いか? */
	rest = interval - t->count;
	if (rest > max_count)
		return max_count;

	/* 残りカウント数を戻す */
	return rest;
}

/*
	タイマを進める (getExecStatesの下請け)
*/
static int foreCount(Z1stat *z1, struct Timer *t, int addition)
{
	int interval, next_count;

	/* タイマが有効でないか? */
	if(!(t->control & 0x8000))
		return FALSE;

	/* カウンタの最大値を得る */
	if(!(t->control & 0x0002) || !(t->control & 0x1000))
		interval = t->interval_a;
	else
		interval = t->interval_b;

	if((z1->timer.t2.control & 0x000c) == 0x0008) { /* タイマ2カウンタ最大でカウンタが進むか? */
		int interval2;

		/* カウンタが進まないか? */
		if(!(z1->timer.t2.control & 0x0002) || !(z1->timer.t2.control & 0x1000))
			interval2 = z1->timer.t2.interval_a;
		else
			interval2 = z1->timer.t2.interval_b;
		if(z1->timer.t2.count + addition < interval2)
			return FALSE;

		/* 最大値に達しないか? */
		next_count = t->count + 1;
		if(t->count < interval) {
			t->count = next_count;
			return FALSE;
		}

		/* カウンタが最大値に達した */
		t->control |= 0x0020;

		/* カウンタを切り替える */
		if(t->control & 0x0002)
			t->control ^= 0x1000;

		/* カウンタを再開または停止させる */
		if(t->control & 0x0001) {
			t->control |= 0x8000;
			t->count = 0;
		} else {
			t->control &= ~0x8000;
			t->count = interval;
		}
	} else {
		/* 最大値に達しないか? */
		next_count = t->count + addition;
		if(next_count < interval) {
			t->count = next_count;
			return FALSE;
		}

		/* カウンタが最大値に達した */
		t->control |= 0x0020;

		/* カウンタを切り替える */
		if(t->control & 0x0002)
			t->control ^= 0x1000;

		/* カウンタを再開または停止させる */
		if(t->control & 0x0001) {
			t->control |= 0x8000;
			if(interval == 0)
				t->count = 0;
			else
				t->count = (next_count - interval) % interval;
		} else
			t->control &= ~0x8000;
	}

	/* 割込が発生するか? */
	return t->control & 0x2000;
}

/*
	I/Oポートに1バイト出力する
*/
int sendIOData(struct IOPort *s, uint8 data)
{
	FILE *fp;
	size_t size;
	char buf[256];

	if(s->pos <= 0) {
		s->pos = 0;
		unlink(s->path);
	}

	if((fp = fopen(s->path, "rb")) == NULL)
		s->pos = 0;
	else
		fclose(fp);

	if((fp = fopen(s->path, "ab")) == NULL)
		return FALSE;
	if((size = fwrite(&data, 1, 1, fp)) > 0)
		s->pos += size;
	fclose(fp);

	sprintf(buf, "%s %dbytes", s->path, s->pos);
	setTitle(buf);
	return size > 0;
}

/*
	I/Oポートから受信するデータを得る(ポインタは進めない)
*/
int peekIOData(struct IOPort *s, uint8 *data)
{
	FILE *fp;
	size_t size;
	uint8 dummy;

	if(s->pos < 0) {
		s->pos++;
		return FALSE;
	}

	if(data == NULL)
		data = &dummy;

	*data = 0;

	if((fp = fopen(s->path, "rb")) == NULL) {
		if((fp = fopen(s->path, "wb")) != NULL)
			fclose(fp);
		return FALSE;
	}

	if(fseek(fp, s->pos, SEEK_SET) > 0)
		size = fread(data, 1, 1, fp);
	else
		size = 0;

	fclose(fp);
	return size > 0;
}

/*
	I/Oポートから受信するデータを得る
*/
int receiveIOData(struct IOPort *s, uint8 *data)
{
	char buf[256];

	if(!peekIOData(s, data))
		return FALSE;
	s->pos++;

	sprintf(buf, "%s %dbytes", s->path, s->pos);
	setTitle(buf);
	return TRUE;
}

/*
	BCDをバイナリに変換する (下請け)
*/
static int decodeBCD(uint8 bcd)
{
	if((bcd & 0x0f) >= 0x0a)
		bcd &= 0xf0;
	if((bcd & 0xf0) >= 0xa0)
		bcd &= 0x0f;

	return (bcd >> 4) * 10 + (bcd & 0x0f);
}

/*
	バイナリをBCDに変換する (下請け)
*/
static uint8 encodeBCD(int val)
{
	if(val < 0)
		val = 100 - (-val % 100);

	return (val / 10) << 4 | (val % 10);
}

/*
	数値の仮数部を得る (opXYの下請け)
*/
int64 decodeMan(const uint8 *bcd)
{
	return
	decodeBCD(bcd[0]) * 1LL +
	decodeBCD(bcd[1]) * 100LL +
	decodeBCD(bcd[2]) * 10000LL +
	decodeBCD(bcd[3]) * 1000000LL +
	decodeBCD(bcd[4]) * 100000000LL +
	decodeBCD(bcd[5]) * 10000000000LL +
	decodeBCD(bcd[6]) * 1000000000000LL;
}

/*
	数値の指数部・符号を得る (opXYの下請け)
*/
static int decodeExpSgn(const uint8 *bcd)
{
	return
	decodeBCD(bcd[7]) * 1 +
	decodeBCD(bcd[8]) * 100;
}

/*
	数値の指数部を得る (opXYの下請け)
*/
static int decodeExp(const uint8 *bcd)
{
	return decodeBCD(bcd[7]);
}

/*
	数値の符号を得る (opXYの下請け)
*/
static int decodeSgn(const uint8 *bcd)
{
	return decodeBCD(bcd[8]);
}

/*
	数値の仮数部を設定する (opXYの下請け)
*/
uint8 *encodeMan(uint8 *bcd, int64 man)
{
	int64 v;

	if(man < 0)
		man = 100000000000000LL + man;

	v = man % 10; bcd[0] = v; man /= 10;
	v = man % 10; bcd[0] |= (v << 4); man /= 10;
	v = man % 10; bcd[1] = v; man /= 10;
	v = man % 10; bcd[1] |= (v << 4); man /= 10;
	v = man % 10; bcd[2] = v; man /= 10;
	v = man % 10; bcd[2] |= (v << 4); man /= 10;
	v = man % 10; bcd[3] = v; man /= 10;
	v = man % 10; bcd[3] |= (v << 4); man /= 10;
	v = man % 10; bcd[4] = v; man /= 10;
	v = man % 10; bcd[4] |= (v << 4); man /= 10;
	v = man % 10; bcd[5] = v; man /= 10;
	v = man % 10; bcd[5] |= (v << 4); man /= 10;
	v = man % 10; bcd[6] = v; man /= 10;
	v = man % 10; bcd[6] |= (v << 4); man /= 10;

	return bcd;
}

/*
	数値の指数部・符号を設定する (opXYの下請け)
*/
static uint8 *encodeExpSgn(uint8 *bcd, int exp_sgn)
{
	int v;

	if(exp_sgn < 0)
		exp_sgn = -exp_sgn;

	v = exp_sgn % 10; bcd[7] = v; exp_sgn /= 10;
	v = exp_sgn % 10; bcd[7] |= (v << 4); exp_sgn /= 10;
	v = exp_sgn % 10; bcd[8] = v; exp_sgn /= 10;
	v = exp_sgn % 10; bcd[8] |= (v << 4); exp_sgn /= 10;

	return bcd;
}

/*
	数値の指数部を設定する (opXYの下請け)
*/
static uint8 *encodeExp(uint8 *bcd, int e)
{
	bcd[7] = encodeBCD(e);

	return bcd;
}

/*
	数値の符号を設定する (opXYの下請け)
*/
static uint8 *encodeSgn(uint8 *bcd, int s)
{
	bcd[8] = encodeBCD(s);

	return bcd;
}

/*
	XとYを演算した結果とフラグを求める
*/
void opXY(uint8 op, const uint8 *x, const uint8 *y, uint8 *a, uint8 *f)
{
	int64 a_man;
	int a_exp;
	const uint8 zero[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	switch(op & 0x02) {
	case 0x00: /* X */
		memcpy(a, x, 9);
		break;
	case 0x02: /* Y */
		memcpy(a, y, 9);
		break;
	}

	switch(op & 0x73) {
	case 0x00: /* X + Y */
		encodeExpSgn(a, (decodeExpSgn(a) + decodeExpSgn(y)) % 10000);
	case 0x40: /* 仮数部 X + Y */
		if((a_man = decodeMan(a) + decodeMan(y)) > 99999999999999LL) {
			a_man -= 100000000000000LL;
			*f = 1;
		} else
			*f = 0;
		encodeMan(a, a_man);
		break;

	case 0x01: /* X - Y */
		encodeExpSgn(a, (10000 + decodeExpSgn(a) - decodeExpSgn(y)) % 10000);
	case 0x41: /* 仮数部 X - Y */
		if((a_man = decodeMan(a) - decodeMan(y)) < 0LL) {
			a_man += 100000000000000LL;
			*f = 1;
		} else
			*f = 0;
		encodeMan(a, a_man);
		break;

	case 0x02: /* Y + X */
		encodeExpSgn(a, (decodeExpSgn(a) + decodeExpSgn(x)) % 10000);
	case 0x42: /* 仮数部 Y + X */
		if((a_man = decodeMan(a) + decodeMan(x)) > 99999999999999LL) {
			a_man -= 100000000000000LL;
			*f = 1;
		} else
			*f = 0;
		encodeMan(a, a_man);
		break;

	case 0x03: /* Y - X */
		encodeExpSgn(a, (10000 + decodeExpSgn(a) - decodeExpSgn(x)) % 10000);
	case 0x43: /* 仮数部 Y - X */
		if((a_man = decodeMan(a) - decodeMan(x)) < 0LL) {
			a_man += 100000000000000LL;
			*f = 1;
		} else
			*f = 0;
		encodeMan(a, a_man);
		break;

	case 0x10: /* 指数部 X + 1 */
	case 0x12: /* 指数部 Y + 1 */
	case 0x30: /* 指数部 X + 1 */
	case 0x32: /* 指数部 Y + 1 */
		if((a_exp = decodeExp(a) + 1) > 99) {
			encodeExp(a, a_exp % 100);
			encodeSgn(a, (decodeSgn(a) + 1) % 100);
		} else
			encodeExp(a, a_exp);
		*f = 0;
		break;

	case 0x11: /* 指数部 X - 1 */
	case 0x13: /* 指数部 Y - 1 */
	case 0x31: /* 指数部 X - 1 */
	case 0x33: /* 指数部 Y - 1 */
		if((a_exp = decodeExp(a) - 1) < 0) {
			encodeExp(a, a_exp + 100);
			encodeSgn(a, (decodeSgn(a) + 100 - 1) % 100);
		} else
			encodeExp(a, a_exp);
		*f = 0;
		break;

	case 0x20: /* 符号 X + 5 */
	case 0x22: /* 符号 Y + 5 */
		encodeSgn(a, (decodeSgn(a) + 5) % 100);
		*f = 0;
		break;

	case 0x21: /* 符号 X - 5 */
	case 0x23: /* 符号 Y - 5 */
		encodeSgn(a, (decodeSgn(a) + 100 - 5) % 100);
		*f = 0;
		break;

	default: /* 不変 */
		*f = 0;
		break;
	}

	if(memcmp(a, zero, sizeof(zero)) == 0) /* 仮数部が0か? */
		*f |= 0x40;
}

/*
	レジスタの値を文字列に変換する
*/
char *decodeReg(char *text, const uint8 *value)
{
	int e = decodeExp(value), s = decodeSgn(value), s_man;
	char buf[64];

	if(s % 5 == 0)
		e = -(100 - e);

	if(s / 5 == 0)
		s_man = 1;
	else
		s_man = -1;

	sprintf(buf, "%s%d.%02d%02d%02d%02d%02d%02dE%d %02x %02x %02x %02x %02x %02x %02x %02x %02x",
	s_man < 0 ? "-": "",
	decodeBCD(value[6]),
	decodeBCD(value[5]),
	decodeBCD(value[4]),
	decodeBCD(value[3]),
	decodeBCD(value[2]),
	decodeBCD(value[1]),
	decodeBCD(value[0]),
	e,
	value[0],
	value[1],
	value[2],
	value[3],
	value[4],
	value[5],
	value[6],
	value[7],
	value[8]);

	strcpy(text, buf);
	return text;
}

/*
	数値を文字列に変換する
*/
char *decodeValue(char *text, const uint8 *value)
{
	int e = decodeBCD(value[7] & 0x0f) * 10 + decodeBCD(value[6] >> 4), s = decodeBCD(value[7] >> 4), s_man, s_exp;
	char buf[32];

	if(s & 0x04) {
		s_man = -1;
		s -= 5;
	} else
		s_man = 1;

	if(s & 0x01)
		s_exp = 1;
	else {
		s_exp = -1;
		e = 100 - e;
	}

	sprintf(buf, "%s%d.%02d%02d%02d%02d%02d%02dE%s%d",
	s_man < 0 ? "-": "",
	decodeBCD(value[6] & 0x0f),
	decodeBCD(value[5]),
	decodeBCD(value[4]),
	decodeBCD(value[3]),
	decodeBCD(value[2]),
	decodeBCD(value[1]),
	decodeBCD(value[0]),
	s_exp < 0 ? "-": "",
	e);

	strcpy(text, buf);
	return text;
}

/*
	音をバッファに書き込む
*/
void writeSound(Z1stat *z1, uint8 buzzer)
{
	struct Sound *sound = &z1->sound;
	long long states;
	int pos;
	int8 *sound_buffer;

	if(sound->len == 0)
		return;

	/*
	states = (z1->cpu.i.total_states - z1->cpu.i.states) % (z1->setting.cpu_clock / z1->setting.refresh_rate);
	pos = AUDIO_RATE * states / z1->setting.cpu_clock;
	*/
	states = MIN(MAX(z1->cpu.i.total_states - sound->last_states - z1->cpu.i.states, 0), z1->setting.cpu_clock / z1->setting.refresh_rate);
	pos = AUDIO_RATE * states / z1->setting.cpu_clock;
	sound_buffer = sound->buffer[sound->page == 0];
/*
	printf("states=%d, pos=%d (%d%%), buzzer=%02x\n", (int )states, pos, pos * 100 / sound->len, buzzer);
*/
/*
	pos = (pos / 8) * 8;
	if(sound->last_pos == pos || sound->last_pos == pos + 1 || sound->last_pos == pos + 2 || sound->last_pos == pos + 3 || sound->last_pos == pos + 4 || sound->last_pos == pos + 5 || sound->last_pos == pos + 6 || sound->last_pos == pos + 7)
		pos += 8;
	else if(sound->last_pos == pos + 8 || sound->last_pos == pos + 9 || sound->last_pos == pos + 10 || sound->last_pos == pos + 11 || sound->last_pos == pos + 12 || sound->last_pos == pos + 13 || sound->last_pos == pos + 14 || sound->last_pos == pos + 15)
		pos += 16;
*/
	if(pos == sound->last_pos)
		pos++;
	else if(pos + 1 == sound->last_pos)
		pos += 2;
	sound->last_pos = pos;
	if(pos < 0)
		pos = 0;

	if(buzzer & 0x02)
		sound->last_vol = sound->last_vol < 0 ? 16: -16;
	if(!(buzzer & 0x01))
		sound->last_vol = 0;
	if(pos < sound->len)
		memset(sound_buffer + pos, sound->last_vol, sound->len - pos);
}

/*
	サウンドバッファを切り替える
*/
void flipSoundBuffer(Z1stat *z1)
{
	struct Sound *sound = &z1->sound;

	if(sound->len == 0)
		return;
/*
	while(!sound->played)
		SDL_Delay(0);
*/

#if 0
	{
		uint8 buf[16384], *p;
		int8 *q;

		for(p = buf, q = sound->buffer[sound->page == 0]; p < buf + sound->len; p++, q++) {
			*p = (int )*q + 128;
		}
		appendWave("sound.wav", buf, sound->len);
		printf("\n");
	}
#endif

	memset(sound->buffer[sound->page], sound->last_vol, sound->len);
	sound->page = (sound->page == 0);
	sound->last_states = z1->cpu.i.total_states;
	sound->last_pos = INT_MIN;
	sound->played = FALSE;
}

/*
	LCDを更新する
*/
static void updateLCD(Z1stat *z1)
{
	int d, *p;
	const uint8 *v;

	/* LCDを更新する */
	if(z1->setting.scales == 2)
		d = 255;
	else
		d = 256 / (z1->setting.refresh_rate / 10);

	p = z1->lcd.pix;
	for(v = z1->vram.vram; v < z1->vram.vram + LCD_HEIGHT * LCD_WIDTH / 8; v++) {
		if(*v & 0x80) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x40) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x20) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x10) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x08) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x04) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x02) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
		if(*v & 0x01) *p = MIN(*p + d, 255); else *p = MAX(*p - d, 0);
		p++;
	}
	if(z1->vram.status & 0x01) z1->lcd.symbol[0] = MIN(z1->lcd.symbol[0] + d, 255); else z1->lcd.symbol[0] = MAX(z1->lcd.symbol[0] - d, 0); /* CAPS */
	if(z1->vram.status & 0x04) z1->lcd.symbol[1] = MIN(z1->lcd.symbol[1] + d, 255); else z1->lcd.symbol[1] = MAX(z1->lcd.symbol[1] - d, 0); /* S */
	if(z1->vram.status & 0x08) z1->lcd.symbol[2] = MIN(z1->lcd.symbol[2] + d, 255); else z1->lcd.symbol[2] = MAX(z1->lcd.symbol[2] - d, 0); /* BASIC */
	if(z1->vram.status & 0x10) z1->lcd.symbol[3] = MIN(z1->lcd.symbol[3] + d, 255); else z1->lcd.symbol[3] = MAX(z1->lcd.symbol[3] - d, 0); /* DEG */
	if(z1->vram.status & 0x20) z1->lcd.symbol[4] = MIN(z1->lcd.symbol[4] + d, 255); else z1->lcd.symbol[4] = MAX(z1->lcd.symbol[4] - d, 0); /* RAD */
	if(z1->vram.status & 0x80) z1->lcd.symbol[5] = MIN(z1->lcd.symbol[5] + d, 255); else z1->lcd.symbol[5] = MAX(z1->lcd.symbol[5] - d, 0); /* GRA */

	updateWindow(z1);
}

/*
	UTF-16をUTF-8に変換する(ankToUtf8_1の下請け)
*/
static inline int utf16ToUtf8_1(char *utf8, uint16 utf16)
{
	if(utf16 < 0x80) {
		utf8[0] = utf16;
		return 1;
	} else if(utf16 < 0x800) {
		utf8[0] = 0xc0 | (utf16 >> 6);
		utf8[1] = 0x80 | (utf16 & 0x3f);
		return 2;
	} else {
		utf8[0] = 0xe0 | (utf16 >> 12);
		utf8[1] = 0x80 | ((utf16 >> 6) & 0x3f);
		utf8[2] = 0x80 | (utf16 & 0x3f);
		return 3;
	}
}

/*
	ANK文字をUTF-8に変換する(ankToUtf8の下請け)
*/
static inline int ankToUtf8_1(char *utf8, const uint8 ank)
{
	if(ank <= 0x7f) {
		*utf8 = ank;
		return 1;
	} else if(0xa1 <= ank && ank <= 0xef)
		return utf16ToUtf8_1(utf8, 0xff60 + ank - 0xa0);
	else {
		*utf8 = '_';
		return 1;
	}
}

/*
	ANK文字列をUTF-8文字列に変換する(getActFileの下請け)
*/
static char *ankToUtf8(char *utf8, const uint8 *ank)
{
	const uint8 *p;
	char *q = utf8;

	for(p = ank; *p != 0; p++)
		q += ankToUtf8_1(q, *p);
	*q = 0;
	return utf8;
}

/*
	フロッピーディスクのファイル名から実際のファイル名を得る
*/
static char *getActFile(char *act_file_utf8, const uint8 *fd_file)
{
	const uint8 *p;
	uint8 act_file_ank[16], *q;

	for(p = fd_file, q = act_file_ank; p < fd_file + 8 && *p != ' '; p++, q++)
		*q = toupper(*p);

	*q++ = '.';

	for(p = fd_file + 8; p < fd_file + 11 && *p != 0; p++, q++)
		*q = toupper(*p);

	*q = 0;

	return ankToUtf8(act_file_utf8, act_file_ank);
}

/*
	UTF-8をUTF-16に変換する(utf8ToAnk_1の下請け)
*/
static inline int utf8ToUtf16(uint16 *utf16, const char *utf8)
{
	if((utf8[0] & 0x80) == 0) {
		*utf16 = *utf8;
		return 1;
	} else if((utf8[0] & 0xe0) == 0xc0) {
		*utf16 = (utf8[0] & 0x1f) << 6U | (utf8[1] & 0x3f);
		return 2;
	} else if((utf8[0] & 0xf0) == 0xe0) {
		*utf16 = (utf8[0] & 0x0fU) << 12U | (utf8[1] & 0x3fU) << 6U | (utf8[2] & 0x3fU);
		return 3;
	} else if((utf8[0] & 0xf8) == 0xf0) {
		*utf16 = 0x20;
		return 4;
	} else if((utf8[0] & 0xfc) == 0xf8) {
		*utf16 = 0x20;
		return 5;
	} else if((utf8[0] & 0xfe) == 0xfc) {
		*utf16 = 0x20;
		return 6;
	} else {
		*utf16 = 0;
		return 1;
	}
}

/*
	UTF-8をANK文字に変換する(utf8ToAnkの下請け)
*/
static inline int utf8ToAnk_1(uint8 *ank, const char *utf8)
{
	int size;
	uint16 utf16;

	size = utf8ToUtf16(&utf16, utf8);
	if(utf16 <= 0x7f)
		*ank = utf16;
	else if(0xff61 <= utf16 && utf16 <= 0xff9f)
		*ank = utf16 - 0xff60 + 0xa0;
	else
		*ank = '_';
	return size;
}

/*
	UTF-8文字列をANK文字列に変換する(getFdFileの下請け)
*/
static uint8 *utf8ToAnk(uint8 *ank, const char *utf8)
{
	const char *p;
	uint8 *q = ank;

	for(p = utf8; *p != 0; p += utf8ToAnk_1(q++, p))
		;
	*q = 0;
	return ank;
}

/*
	実際のファイル名からフロッピーディスクのファイル名を得る
*/
static int getFdFile(uint8 *fd_file, const char *act_file_utf8)
{
	uint8 act_file_ank[PATH_MAX], *p;
	const uint8 *q;

	utf8ToAnk(act_file_ank, act_file_utf8);

	strcpy((char *)fd_file, "           ");

	for(p = fd_file, q = act_file_ank; p < fd_file + 8 && *q != ' ' && *q != '.' && *q != 0; p++, q++) {
#ifdef _WIN32
		if(isalpha((unsigned )*q))
			*p = toupper(*q);
		else
			*p = *q;
#else
		if(isalpha((unsigned )*q) && islower((unsigned )*q))
			return FALSE;
		*p = *q;
#endif
	}
	if(*q == 0)
		return TRUE;
	if(*q != '.')
		return FALSE;
	q++;

	for(p = fd_file + 8; p < fd_file + 11 && *q != ' ' && *q != '.' && *q != 0; p++, q++) {
#ifdef _WIN32
		if(isalpha((unsigned )*q))
			*p = toupper(*q);
		else
			*p = *q;
#else
		if(isalpha((unsigned )*q) && islower((unsigned )*q))
			return FALSE;
		*p = *q;
#endif
	}
	if(*q != 0)
		return FALSE;
	return TRUE;
}

/*
	フロッピーディスクのファイル名から実際のパス名を得る
*/
static char *getPath(const struct Disk *disk, char *path, const uint8 *fd_file)
{
	char dir[PATH_MAX], file[PATH_MAX];

	setHomeDir(dir, disk->dir);
	getActFile(file, fd_file);
	sprintf(path, "%s/%s", dir, file);
	return path;
}

/*
	ファイルの属性を得る
*/
static uint8 getFileAttr(const char *path)
{
	FILE *fp;
	uint8 buf[8];

	if((fp = fopen(path, "rb")) < 0)
		return '?';
	memset(buf, 0, 2);
	fread(buf, 1, 2, fp);
	fclose(fp);

	return (memcmp(buf, "\xff\xfe", 2) == 0 ? 'B': 'A');
}

/*
	フロッピーディスクの空き容量を得る
*/
uint8 getFdFreeSize(struct Disk *disk, uint32 *size)
{
#ifdef WIN32
	ULARGE_INTEGER free, avail, total;
	wchar_t wdir[PATH_MAX], wroot[PATH_MAX], *wfile;

	MultiByteToWideChar(CP_UTF8, 0, disk->dir, -1, wdir, sizeof(wdir));
	GetFullPathNameW(wdir, PATH_MAX, wroot, &wfile);
	wroot[2] = 0;

	if(!GetDiskFreeSpaceExW(wroot, &free, &avail, &total))
		return 0x03; /* NR error */
	else if(free.QuadPart / 1024 > 0xffffULL)
		*size = 0xffff;
	else
		*size = free.QuadPart / 1024;
#else
	struct statvfs stat;
	unsigned long long free;
	char root[PATH_MAX];

	if(disk->dir[0] == '~')
		sprintf(root, "%s%s", getenv("HOME"), &disk->dir[1]);
	else
		strcpy(root, disk->dir);

	if(statvfs(root, &stat) < 0)
		return 0x03; /* NR error */
	if((free = stat.f_frsize * stat.f_blocks / 1024) > 0xffffULL)
		*size = 0xffff;
	else
		*size = free;
#endif
	return 0x00;
}

/*
	フロッピーディスクのファイルをオープンする
*/
uint8 openFdFile(struct Disk *disk, const uint8 *file, const uint8 *mode)
{
	char path[PATH_MAX];
	struct stat s;

	if(disk->fp != NULL) {
		fclose(disk->fp);
		disk->fp = NULL;
		return 0x03; /* NR error */
	}

	getPath(disk, path, file);

	if(*mode == 'R')
		disk->fp = fopen(path, "rb");
	else if(*mode == 'W')
		disk->fp = fopen(path, "wb");
	else if(*mode == 'A')
		disk->fp = fopen(path, "ab");
	else
		disk->fp = NULL;
	if(disk->fp == NULL)
		return errno == ENOENT ? 0x02/* FL error */: 0x07/* PR error */;

	if(stat(path, &s))
		return errno == ENOENT ? 0x02/* FL error */: 0x07/* PR error */;
	disk->size = s.st_size;

	disk->pos = 0;
	return 0x00;
}

/*
	フロッピーディスクのファイルをクローズする
*/
uint8 closeFdFile(struct Disk *disk)
{
	if(disk->fp == NULL)
		return 0x00;

	fclose(disk->fp);
	disk->fp = NULL;
	return 0x00;
}

/*
	フロッピーディスクのファイルを読み込む
*/
uint8 readFdFile(struct Disk *disk, uint8 *data)
{
	if(disk->fp == NULL)
		return 0x03; /* NR error */

	if(fread(data, 1, 1, disk->fp) <= 0) {
		fclose(disk->fp);
		disk->fp = NULL;
		return 0x0b; /* DF error */
	}
	disk->pos++;
	return 0x00;
}

/*
	フロッピーディスクのファイルに書き込む
*/
uint8 writeFdFile(struct Disk *disk, uint8 data, const uint8 *size)
{
	if(disk->fp == NULL)
		return 0x00;

	if(fwrite(&data, 1, 1, disk->fp) <= 0) {
		fclose(disk->fp);
		disk->fp = NULL;
		return 0x07; /* PR error */
	}
	disk->pos++;

	if(size != NULL) {
		size_t last_pos = (size_t )size[0] | ((size_t )size[1] << 8) | ((size_t )size[2] << 16) | ((size_t )size[3] << 24);

		if(disk->pos >= last_pos) {
			fclose(disk->fp);
			disk->fp = NULL;
		}
	}
	return 0x00;
}

/*
	フロッピーディスクのファイル名を変更する
*/
uint8 renameFdFile(struct Disk *disk, const uint8 *old_file, const uint8 *new_file)
{
	char old_path[PATH_MAX], new_path[PATH_MAX];

	if(rename(getPath(disk, old_path, old_file), getPath(disk, new_path, new_file)) < 0)
		return errno == ENOENT ? 0x02/* FL error */: 0x07/* PR error */;
	return 0x00;
}

/*
	フロッピーディスクのファイルを削除する
*/
uint8 deleteFdFile(struct Disk *disk, const uint8 *file)
{
	char path[PATH_MAX];
	if(unlink(getPath(disk, path, file)) < 0)
		return errno == ENOENT ? 0x02/* FL error */: 0x07/* PR error */;
	return 0x00;
}

#ifndef _WIN32
/*
	フロッピーディスクのファイルがワイルドカードと一致するかチェックする (findFdFileの下請け)
*/
static int matchFile(const uint8 *file, const uint8 *wild)
{
	const uint8 *p, *q;

	for(p = file, q = wild; p < file + 11; p++, q++)
		if(*p != *q && *q != '?')
			return FALSE;
	return TRUE;
}
#endif

/*
	フロッピーディスクのファイルを検索する
*/
uint8 findFdFile(struct Disk *disk, const uint8 *wild, uint8 *data)
{
#ifdef _WIN32
	int n = 0;
	char path[PATH_MAX], dir[PATH_MAX], file[PATH_MAX];
	uint8 *p;
	wchar_t wpath[PATH_MAX];
	HANDLE h;
	WIN32_FIND_DATAW f;

	if(disk->files == NULL)
		disk->files = malloc(1);
	else
		disk->files = realloc(disk->files, 1);

	setHomeDir(dir, disk->dir);
	MultiByteToWideChar(CP_UTF8, 0, getPath(disk, path, wild), -1, wpath, sizeof(wpath));
	if((h = FindFirstFileW(wpath, &f)) == INVALID_HANDLE_VALUE) {
		disk->files_p = NULL;
		*data = 0x00;
		return 0x03;
	}

	do {
		if(f.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN))
			continue;
		if(f.nFileSizeLow == 0 && f.nFileSizeHigh == 0)
			continue;
		if(f.nFileSizeHigh)
			continue;

		WideCharToMultiByte(CP_UTF8, 0, f.cFileName, -1, (LPSTR )file, sizeof(file), NULL, NULL);
		sprintf(path, "%s\\%s", dir, file);

		disk->files = realloc(disk->files, 20 * n + 20);
		p = disk->files + n * 20;
		if(!getFdFile(p, file))
			continue;
		p += 11;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = f.nFileSizeLow & 0xff;
		*p++ = (f.nFileSizeLow >> 8) & 0xff;
		*p++ = (f.nFileSizeLow >> 16) & 0xff;
		*p++ = (f.nFileSizeLow >> 24) & 0xff;
		*p++ = getFileAttr(path);
		n++;
	} while(n < 0xff && FindNextFileW(h, &f));

	FindClose(h);

	disk->files_p = disk->files;
	*data = n;
	return 0x00;
#else
	int n = 0;
	char path[PATH_MAX], dir[PATH_MAX];
	uint8 *p;
	DIR *d;
	struct dirent *f;
	struct stat s;

	if(disk->files == NULL)
		disk->files = malloc(1);
	else
		disk->files = realloc(disk->files, 1);

	setHomeDir(dir, disk->dir);
	if((d = opendir(dir)) == NULL) {
		disk->files_p = NULL;
		*data = 0x00;
		return 0x03;
	}

	for(f = readdir(d); f != NULL; f = readdir(d)) {
		if(f->d_type == DT_DIR || f->d_name[0] == '.')
			continue;

		sprintf(path, "%s/%s", dir, f->d_name);
		stat(path, &s);
		if(s.st_size == 0 || s.st_size > 0xffffffff)
			continue;

		disk->files = realloc(disk->files, 20 * n + 20);
		p = disk->files + n * 20;
		if(!getFdFile(p, f->d_name))
			continue;
		if(!matchFile(p, wild))
			continue;
		p += 11;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = 0x00;
		*p++ = s.st_size & 0xff;
		*p++ = (s.st_size >> 8) & 0xff;
		*p++ = (s.st_size >> 16) & 0xff;
		*p++ = (s.st_size >> 24) & 0xff;
		*p++ = getFileAttr(path);
		n++;
	}

	closedir(d);

	disk->files_p = disk->files;
	*data = n;
	return 0x00;
#endif
}

/*
	フロッピーディスクのファイルの検索結果を得る
*/
uint8 getFoundFdFile(struct Disk *disk, uint8 *data)
{
	if(disk->files_p == NULL)
		return 0x03;

	*data = *disk->files_p++;
	return 0x00;
}

/*
	フロッピーディスクをフォーマットする
*/
uint8 formatFd(struct Disk *disk)
{
	/* 何もしない */
	return 0x00;
}

/*
	シリアルポートに/から送信/受信するデータを設定する
*/
void setIOData(struct IOPort *s, const char *path)
{
	char buf[256];

	if(path != NULL)
		strcpy(s->path, path);
	s->pos = -4;

	sprintf(buf, "%s %dbytes", s->path, 0);
	setTitle(buf);
}

/*
	自動入力キューの末尾のポインタを得る(下請け)
*/
static uint8 *getAutoKeyLast(struct AutoKey *auto_key)
{
	if(auto_key->key == NULL) {
		auto_key->press = FALSE;
		auto_key->key = malloc(2);
		return auto_key->key;
	} else {
		int len = strlen((char *)auto_key->key);
		auto_key->key = realloc(auto_key->key, len + 2);
		return auto_key->key + len;
	}
}

/*
	自動入力するキーを設定する
*/
void setAutoKey(Z1stat *z1, uint8 key)
{
	uint8 *last = getAutoKeyLast(&z1->auto_key);

	*last++ = key;
	*last = 0;
}

/*
	自動入力する文字を設定する
*/
void setAutoChr(Z1stat *z1, uint8 ank)
{
	const static uint8 none[] = { 0 };
	const static uint8 ltop[] = { ZKEY_2ND, ZKEY_LEFT, 0 }; /* L.TOP */
	const static uint8 lcan[] = { ZKEY_2ND, ZKEY_BS, 0 }; /* L.CAN */
	const static uint8 lend[] = { ZKEY_2ND, ZKEY_RIGHT, 0 }; /* L.END */
	const static uint8 bs[] = { ZKEY_BS, 0 }; /* バックスペース */
	const static uint8 tab[] = { ZKEY_TAB, 0 }; /* タブ */
	const static uint8 home[] = { ZKEY_2ND, ZKEY_CLS, 0 }; /* ホーム */
	const static uint8 cls[] = { ZKEY_CLS, 0 }; /* クリア */
	const static uint8 del[] = { ZKEY_DEL, 0 }; /* 削除 */
	const static uint8 ins[] = { ZKEY_INS, 0 }; /* 挿入 */
	const static uint8 right[] = { ZKEY_RIGHT, 0 }; /* 右 */
	const static uint8 left[] = { ZKEY_LEFT, 0 }; /* 左 */
	const static uint8 up[] = { ZKEY_UP, 0 }; /* 上 */
	const static uint8 down[] = { ZKEY_DOWN, 0 }; /* 下 */
	const static uint8 cr[] = { ZKEY_RETURN, 0 }; /* 改行 */
	const static uint8 space[] = { ZKEY_SPC, 0 }; /* 空白 */
	const static uint8 exclaim[] = { ZKEY_2ND, ZKEY_Q, 0 }; /* ! */
	const static uint8 dblquote[] = { ZKEY_2ND, ZKEY_W, 0 }; /* " */
	const static uint8 hash[] = { ZKEY_2ND, ZKEY_E, 0 }; /* # */
	const static uint8 dollar[] = { ZKEY_2ND, ZKEY_R, 0 }; /* $ */
	const static uint8 percent[] = { ZKEY_2ND, ZKEY_T, 0 }; /* % */
	const static uint8 ampersand[] = { ZKEY_2ND, ZKEY_Y, 0 }; /* & */
	const static uint8 quote[] = { ZKEY_2ND, ZKEY_U, 0 }; /* ' */
	const static uint8 lparen[] = { ZKEY_LPAREN, 0 }; /* ( */
	const static uint8 rparen[] = { ZKEY_RPAREN, 0 }; /* ) */
	const static uint8 aster[] = { ZKEY_ASTER, 0 }; /* * */
	const static uint8 plus[] = { ZKEY_PLUS, 0 }; /* + */
	const static uint8 comma[] = { ZKEY_COMMA, 0 }; /* , */
	const static uint8 minus[] = { ZKEY_MINUS, 0 }; /* - */
	const static uint8 period[] = { ZKEY_PERIOD, 0 }; /* . */
	const static uint8 slash[] = { ZKEY_SLASH, 0 }; /* / */
	const static uint8 zero[] = { ZKEY_0, 0 }; /* 0 */
	const static uint8 one[] = { ZKEY_1, 0 }; /* 1 */
	const static uint8 two[] = { ZKEY_2, 0 }; /* 2 */
	const static uint8 three[] = { ZKEY_3, 0 }; /* 3 */
	const static uint8 four[] = { ZKEY_4, 0 }; /* 4 */
	const static uint8 five[] = { ZKEY_5, 0 }; /* 5 */
	const static uint8 six[] = { ZKEY_6, 0 }; /* 6 */
	const static uint8 seven[] = { ZKEY_7, 0 }; /* 7 */
	const static uint8 eight[] = { ZKEY_8, 0 }; /* 8 */
	const static uint8 nine[] = { ZKEY_9, 0 }; /* 9 */
	const static uint8 colon[] = { ZKEY_COLON, 0 }; /* : */
	const static uint8 semicolon[] = { ZKEY_SEMICOLON, 0 }; /* ; */
	const static uint8 less[] = { ZKEY_2ND, ZKEY_K, 0 }; /* < */
	const static uint8 equal[] = { ZKEY_EQUAL, 0 }; /* = */
	const static uint8 greater[] = { ZKEY_2ND, ZKEY_L, 0 }; /* > */
	const static uint8 question[] = { ZKEY_2ND, ZKEY_D, 0 }; /* ? */
	const static uint8 at[] = { ZKEY_2ND, ZKEY_A, 0 }; /* @ */
	const static uint8 a[] = { ZKEY_A, 0 }; /* A */
	const static uint8 b[] = { ZKEY_B, 0 }; /* B */
	const static uint8 c[] = { ZKEY_C, 0 }; /* C */
	const static uint8 d[] = { ZKEY_D, 0 }; /* D */
	const static uint8 e[] = { ZKEY_E, 0 }; /* E */
	const static uint8 f[] = { ZKEY_F, 0 }; /* F */
	const static uint8 g[] = { ZKEY_G, 0 }; /* G */
	const static uint8 h[] = { ZKEY_H, 0 }; /* H */
	const static uint8 i[] = { ZKEY_I, 0 }; /* I */
	const static uint8 j[] = { ZKEY_J, 0 }; /* J */
	const static uint8 k[] = { ZKEY_K, 0 }; /* K */
	const static uint8 l[] = { ZKEY_L, 0 }; /* L */
	const static uint8 m[] = { ZKEY_M, 0 }; /* M */
	const static uint8 n[] = { ZKEY_N, 0 }; /* N */
	const static uint8 o[] = { ZKEY_O, 0 }; /* O */
	const static uint8 p[] = { ZKEY_P, 0 }; /* P */
	const static uint8 q[] = { ZKEY_Q, 0 }; /* Q */
	const static uint8 r[] = { ZKEY_R, 0 }; /* R */
	const static uint8 s[] = { ZKEY_S, 0 }; /* S */
	const static uint8 t[] = { ZKEY_T, 0 }; /* T */
	const static uint8 u[] = { ZKEY_U, 0 }; /* U */
	const static uint8 v[] = { ZKEY_V, 0 }; /* V */
	const static uint8 w[] = { ZKEY_W, 0 }; /* W */
	const static uint8 x[] = { ZKEY_X, 0 }; /* X */
	const static uint8 y[] = { ZKEY_Y, 0 }; /* Y */
	const static uint8 z[] = { ZKEY_Z, 0 }; /* Z */
	const static uint8 lbracket[] = { ZKEY_2ND, ZKEY_H, 0 }; /* [ */
	const static uint8 yen[] = { ZKEY_2ND, ZKEY_I, 0 }; /* \ */
	const static uint8 rbracket[] = { ZKEY_2ND, ZKEY_J, 0 }; /* ] */
	const static uint8 hat[] = { ZKEY_HAT, 0 }; /* ^ */
	const static uint8 underbar[] = { ZKEY_2ND, ZKEY_SEMICOLON, 0 }; /* _ */
	const static uint8 backquote[] = { ZKEY_2ND, ZKEY_P, 0 }; /* ` */
	const static uint8 small_a[] = { ZKEY_CAPS, ZKEY_A, ZKEY_CAPS, 0 }; /* a */
	const static uint8 small_b[] = { ZKEY_CAPS, ZKEY_B, ZKEY_CAPS, 0 }; /* b */
	const static uint8 small_c[] = { ZKEY_CAPS, ZKEY_C, ZKEY_CAPS, 0 }; /* c */
	const static uint8 small_d[] = { ZKEY_CAPS, ZKEY_D, ZKEY_CAPS, 0 }; /* d */
	const static uint8 small_e[] = { ZKEY_CAPS, ZKEY_E, ZKEY_CAPS, 0 }; /* e */
	const static uint8 small_f[] = { ZKEY_CAPS, ZKEY_F, ZKEY_CAPS, 0 }; /* f */
	const static uint8 small_g[] = { ZKEY_CAPS, ZKEY_G, ZKEY_CAPS, 0 }; /* g */
	const static uint8 small_h[] = { ZKEY_CAPS, ZKEY_H, ZKEY_CAPS, 0 }; /* h */
	const static uint8 small_i[] = { ZKEY_CAPS, ZKEY_I, ZKEY_CAPS, 0 }; /* i */
	const static uint8 small_j[] = { ZKEY_CAPS, ZKEY_J, ZKEY_CAPS, 0 }; /* j */
	const static uint8 small_k[] = { ZKEY_CAPS, ZKEY_K, ZKEY_CAPS, 0 }; /* k */
	const static uint8 small_l[] = { ZKEY_CAPS, ZKEY_L, ZKEY_CAPS, 0 }; /* l */
	const static uint8 small_m[] = { ZKEY_CAPS, ZKEY_M, ZKEY_CAPS, 0 }; /* m */
	const static uint8 small_n[] = { ZKEY_CAPS, ZKEY_N, ZKEY_CAPS, 0 }; /* n */
	const static uint8 small_o[] = { ZKEY_CAPS, ZKEY_O, ZKEY_CAPS, 0 }; /* o */
	const static uint8 small_p[] = { ZKEY_CAPS, ZKEY_P, ZKEY_CAPS, 0 }; /* p */
	const static uint8 small_q[] = { ZKEY_CAPS, ZKEY_Q, ZKEY_CAPS, 0 }; /* q */
	const static uint8 small_r[] = { ZKEY_CAPS, ZKEY_R, ZKEY_CAPS, 0 }; /* r */
	const static uint8 small_s[] = { ZKEY_CAPS, ZKEY_S, ZKEY_CAPS, 0 }; /* s */
	const static uint8 small_t[] = { ZKEY_CAPS, ZKEY_T, ZKEY_CAPS, 0 }; /* t */
	const static uint8 small_u[] = { ZKEY_CAPS, ZKEY_U, ZKEY_CAPS, 0 }; /* u */
	const static uint8 small_v[] = { ZKEY_CAPS, ZKEY_V, ZKEY_CAPS, 0 }; /* v */
	const static uint8 small_w[] = { ZKEY_CAPS, ZKEY_W, ZKEY_CAPS, 0 }; /* w */
	const static uint8 small_x[] = { ZKEY_CAPS, ZKEY_X, ZKEY_CAPS, 0 }; /* x */
	const static uint8 small_y[] = { ZKEY_CAPS, ZKEY_Y, ZKEY_CAPS, 0 }; /* y */
	const static uint8 small_z[] = { ZKEY_CAPS, ZKEY_Z, ZKEY_CAPS, 0 }; /* z */
	const static uint8 lbrace[] = { ZKEY_2ND, ZKEY_F, 0 }; /* { */
	const static uint8 pipe[] = { ZKEY_2ND, ZKEY_O, 0 }; /* | */
	const static uint8 rbrace[] = { ZKEY_2ND, ZKEY_G, 0 }; /* } */
	const static uint8 tilda[] = { ZKEY_2ND, ZKEY_S, 0 }; /* ~ */
	const static uint8 kuten[] = { ZKEY_PERIOD, 0 }; /* 。 */
	const static uint8 lkakko[] = { ZKEY_2ND, ZKEY_H, 0 }; /* 「 */
	const static uint8 touten[] = { ZKEY_COMMA, 0 }; /* 、 */
	const static uint8 rkakko[] = { ZKEY_2ND, ZKEY_J, 0 }; /* 」 */
	const static uint8 nakaten[] = { ZKEY_PERIOD, 0 }; /* ・ */
	const static uint8 wo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_W, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヲ */
	const static uint8 xa[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ァ */
	const static uint8 xi[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ィ */
	const static uint8 xu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ゥ */
	const static uint8 xe[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ェ */
	const static uint8 xo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ォ */
	const static uint8 xya[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_Y, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ャ */
	const static uint8 xyu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_Y, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ュ */
	const static uint8 xyo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_Y, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ョ */
	const static uint8 xtu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_X, ZKEY_T, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ッ */
	const static uint8 choon[] = { ZKEY_MINUS, 0 }; /* ー */
	const static uint8 aa[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ア */
	const static uint8 ii[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* イ */
	const static uint8 uu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ウ */
	const static uint8 ee[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* エ */
	const static uint8 oo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* オ */
	const static uint8 ka[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_K, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* カ */
	const static uint8 ki[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_K, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* キ */
	const static uint8 ku[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_K, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ク */
	const static uint8 ke[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_K, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ケ */
	const static uint8 ko[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_K, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* コ */
	const static uint8 sa[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_S, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* サ */
	const static uint8 si[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_S, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* シ */
	const static uint8 su[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_S, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ス */
	const static uint8 se[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_S, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* セ */
	const static uint8 so[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_S, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ソ */
	const static uint8 ta[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_T, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* タ */
	const static uint8 ti[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_T, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* チ */
	const static uint8 tu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_T, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ツ */
	const static uint8 te[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_T, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* テ */
	const static uint8 to[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_T, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ト */
	const static uint8 na[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ナ */
	const static uint8 ni[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ニ */
	const static uint8 nu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヌ */
	const static uint8 ne[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ネ */
	const static uint8 no[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ノ */
	const static uint8 ha[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_H, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ハ */
	const static uint8 hi[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_H, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヒ */
	const static uint8 hu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_H, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* フ */
	const static uint8 he[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_H, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヘ */
	const static uint8 ho[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_H, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ホ */
	const static uint8 ma[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_M, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* マ */
	const static uint8 mi[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_M, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ミ */
	const static uint8 mu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_M, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ム */
	const static uint8 me[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_M, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* メ */
	const static uint8 mo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_M, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* モ */
	const static uint8 ya[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_Y, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヤ */
	const static uint8 yu[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_Y, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ユ */
	const static uint8 yo[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_Y, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ヨ */
	const static uint8 ra[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_R, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ラ */
	const static uint8 ri[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_R, ZKEY_I, ZKEY_2ND, ZKEY_CAPS, 0 }; /* リ */
	const static uint8 ru[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_R, ZKEY_U, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ル */
	const static uint8 re[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_R, ZKEY_E, ZKEY_2ND, ZKEY_CAPS, 0 }; /* レ */
	const static uint8 ro[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_R, ZKEY_O, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ロ */
	const static uint8 wa[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_W, ZKEY_A, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ワ */
	const static uint8 nn[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_N, ZKEY_N, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ン */
	const static uint8 dakuon[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_B, ZKEY_A, ZKEY_LEFT, ZKEY_BS, ZKEY_RIGHT, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ゛ */
	const static uint8 handakuon[] = { ZKEY_2ND, ZKEY_CAPS, ZKEY_P, ZKEY_A, ZKEY_LEFT, ZKEY_BS, ZKEY_RIGHT, ZKEY_2ND, ZKEY_CAPS, 0 }; /* ゜ */
	const static uint8 *table[] = {
		none, none, ltop, none, none, lcan, lend, none,
		bs, tab, none, home, cls, cr, none, none,
		none, del, ins, none, none, none, none, none,
		none, none, none, none, right, left, up, down,
		space, exclaim, dblquote, hash, dollar, percent, ampersand, quote,
		lparen, rparen, aster, plus, comma, minus, period, slash,
		zero, one, two, three, four, five, six, seven,
		eight, nine, colon, semicolon, less, equal, greater, question,
		at, a, b, c, d, e, f, g,
		h, i, j, k, l, m, n, o,
		p, q, r, s, t, u, v, w,
		x, y, z, lbracket, yen, rbracket, hat, underbar,
		backquote, small_a, small_b, small_c, small_d, small_e, small_f, small_g,
		small_h, small_i, small_j, small_k, small_l, small_m, small_n, small_o,
		small_p, small_q, small_r, small_s, small_t, small_u, small_v, small_w,
		small_x, small_y, small_z, lbrace, pipe, rbrace, tilda, none,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none,
		none, kuten, lkakko, rkakko, touten, nakaten, wo, xa,
		xi, xu, xe, xo, xya, xyu, xyo, xtu,
		choon, aa, ii, uu, ee, oo, ka, ki,
		ku, ke, ko, sa, si, su, se, so,
		ta, ti, tu, te, to, na, ni, nu,
		ne, no, ha, hi, hu, he, ho, ma,
		mi, mu, me, mo, ya, yu, yo, ra,
		ri, ru, re, ro, wa, nn, dakuon, handakuon,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none,
		none, none, none, none, none, none, none, none
	};
	const uint8 *key;

	for(key = table[ank]; *key != 0; key++)
		setAutoKey(z1, *key);
}

/*
	全角文字を含むUTF-16文字をANK文字に変換する(setAutoTextの下請け)
*/
static int fullToHalf_1(uint8 *ank, uint16 utf16)
{
	const static uint8 kana[][2] = {
		{ (uint8 )0xa7 }, /* ァ */
		{ (uint8 )0xb1 }, /* ア */
		{ (uint8 )0xa8 }, /* ィ */
		{ (uint8 )0xb2 }, /* イ */
		{ (uint8 )0xa9 }, /* ゥ */
		{ (uint8 )0xb3 }, /* ウ */
		{ (uint8 )0xaa }, /* ェ */
		{ (uint8 )0xb4 }, /* エ */
		{ (uint8 )0xab }, /* ォ */
		{ (uint8 )0xb5 }, /* オ */
		{ (uint8 )0xb6 }, /* カ */
		{ (uint8 )0xb6, (uint8 )0xde }, /* ガ */
		{ (uint8 )0xb7 }, /* キ */
		{ (uint8 )0xb7, (uint8 )0xde }, /* ギ */
		{ (uint8 )0xb8 }, /* ク */
		{ (uint8 )0xb8, (uint8 )0xde }, /* グ */
		{ (uint8 )0xb9 }, /* ケ */
		{ (uint8 )0xb9, (uint8 )0xde }, /* ゲ */
		{ (uint8 )0xba }, /* コ */
		{ (uint8 )0xba, (uint8 )0xde }, /* ゴ */
		{ (uint8 )0xbb }, /* サ */
		{ (uint8 )0xbb, (uint8 )0xde }, /* ザ */
		{ (uint8 )0xbc }, /* シ */
		{ (uint8 )0xbc, (uint8 )0xde }, /* ジ */
		{ (uint8 )0xbd }, /* ス */
		{ (uint8 )0xbd, (uint8 )0xde }, /* ズ */
		{ (uint8 )0xbe }, /* セ */
		{ (uint8 )0xbe, (uint8 )0xde }, /* ゼ */
		{ (uint8 )0xbf }, /* ソ */
		{ (uint8 )0xbf, (uint8 )0xde }, /* ゾ */
		{ (uint8 )0xc0 }, /* タ */
		{ (uint8 )0xc0, (uint8 )0xde }, /* ダ */
		{ (uint8 )0xc1 }, /* チ */
		{ (uint8 )0xc1, (uint8 )0xde }, /* ヂ */
		{ (uint8 )0xaf }, /* ッ */
		{ (uint8 )0xc2 }, /* ツ */
		{ (uint8 )0xc2, (uint8 )0xde }, /* ヅ */
		{ (uint8 )0xc3 }, /* テ */
		{ (uint8 )0xc3, (uint8 )0xde }, /* デ */
		{ (uint8 )0xc4 }, /* ト */
		{ (uint8 )0xc4, (uint8 )0xde }, /* ド */
		{ (uint8 )0xc5 }, /* ナ */
		{ (uint8 )0xc6 }, /* ニ */
		{ (uint8 )0xc7 }, /* ヌ */
		{ (uint8 )0xc8 }, /* ネ */
		{ (uint8 )0xc9 }, /* ノ */
		{ (uint8 )0xca }, /* ハ */
		{ (uint8 )0xca, (uint8 )0xde }, /* バ */
		{ (uint8 )0xca, (uint8 )0xdf }, /* パ */
		{ (uint8 )0xcb }, /* ヒ */
		{ (uint8 )0xcb, (uint8 )0xde }, /* ビ */
		{ (uint8 )0xcb, (uint8 )0xdf }, /* ピ */
		{ (uint8 )0xcc }, /* フ */
		{ (uint8 )0xcc, (uint8 )0xde }, /* ブ */
		{ (uint8 )0xcc, (uint8 )0xdf }, /* プ */
		{ (uint8 )0xcd }, /* ヘ */
		{ (uint8 )0xcd, (uint8 )0xde }, /* ベ */
		{ (uint8 )0xcd, (uint8 )0xdf }, /* ペ */
		{ (uint8 )0xce }, /* ホ */
		{ (uint8 )0xce, (uint8 )0xde }, /* ボ */
		{ (uint8 )0xce, (uint8 )0xdf }, /* ポ */
		{ (uint8 )0xcf }, /* マ */
		{ (uint8 )0xd0 }, /* ミ */
		{ (uint8 )0xd1 }, /* ム */
		{ (uint8 )0xd2 }, /* メ */
		{ (uint8 )0xd3 }, /* モ */
		{ (uint8 )0xac }, /* ャ */
		{ (uint8 )0xd4 }, /* ヤ */
		{ (uint8 )0xad }, /* ュ */
		{ (uint8 )0xd5 }, /* ユ */
		{ (uint8 )0xae }, /* ョ */
		{ (uint8 )0xd6 }, /* ヨ */
		{ (uint8 )0xd7 }, /* ラ */
		{ (uint8 )0xd8 }, /* リ */
		{ (uint8 )0xd9 }, /* ル */
		{ (uint8 )0xda }, /* レ */
		{ (uint8 )0xdb }, /* ロ */
		{ (uint8 )0 }, /* ヮ */
		{ (uint8 )0xdc }, /* ワ */
		{ (uint8 )0 }, /* ヰ */
		{ (uint8 )0 }, /* ヱ */
		{ (uint8 )0xa6 }, /* ヲ */
		{ (uint8 )0xdd }, /* ン */
		{ (uint8 )0xb3, (uint8 )0xde } /* ヴ */
	};

	if(utf16 <= 0x7f) { /* ASCII */
		ank[0] = utf16;
		return 1;
	} else if(utf16 == 0x3001) { /* 、 */
		ank[0] = 0xa4;
		return 1;
	} else if(utf16 == 0x300c) { /* 「 */
		ank[0] = 0xa2;
		return 1;
	} else if(utf16 == 0x300d) { /* 」 */
		ank[0] = 0xa3;
		return 1;
	} else if(0x3041 <= utf16 && utf16 <= 0x3094) { /* ひらがな */
		ank[0] = kana[utf16 - 0x3041][0];
		if((ank[1] = kana[utf16 - 0x3041][1]) == 0)
			return 1;
		else
			return 2;
	} else if(utf16 == 0x309b) { /* ゛ */
		ank[0] = 0xee;
		return 1;
	} else if(utf16 == 0x309c) { /* ゜ */
		ank[0] = 0xef;
		return 1;
	} else if(0x30a1 <= utf16 && utf16 <= 0x30f4) { /* カタカナ */
		ank[0] = kana[utf16 - 0x30a1][0];
		if((ank[1] = kana[utf16 - 0x30a1][1]) == 0)
			return 1;
		else
			return 2;
	} else if(utf16 == 0x30fb) { /* ・ */
		ank[0] = 0xa5;
		return 1;
	} else if(utf16 == 0x30fc) { /* ー */
		ank[0] = 0xb0;
		return 1;
	} else if(utf16 == 0x8142) { /* 。 */
		ank[0] = 0xa1;
		return 1;
	} else if(0xff01 <= utf16 && utf16 <= 0xff5e) { /* 全角英数記号 */
		ank[0] = utf16 - 0xff00 + 0x20;
		return 1;
	} else if(0xff61 <= utf16 && utf16 <= 0xff9f) { /* 半角カナ */
		ank[0] = utf16 - 0xff60 + 0xa0;
		return 1;
	} else { /* 変換不能 */
		ank[0] = 0x20;
		return 1;
	}
}

/*
	自動入力する文字列を設定する
*/
void setAutoText(Z1stat *z1, const char *utf8)
{
	int size, len;
	uint16 utf16;
	const char *p;
	uint8 ank[8];

	for(p = utf8; *p != 0; p += size) {
		size = utf8ToUtf16(&utf16, p);
		len = fullToHalf_1(ank, utf16);
		if(len >= 1)
			setAutoChr(z1, ank[0]);
		if(len >= 2)
			setAutoChr(z1, ank[1]);
	}
}

/*
	キーを押す
*/
int pressKey(Z1stat *z1, uint8 key)
{
	if(key == 0)
		return -1;
	if(key == ZKEY_DEBUG) {
		z1->cpu.i.trace = !z1->cpu.i.trace;
		return -1;
	}
	if(key == ZKEY_REWIND_INPORT) {
		setIOData(&z1->rs_receive, NULL);
		return -1;
	}
	if(key == ZKEY_REWIND_OUTPORT) {
		setIOData(&z1->rs_send, NULL);
		return -1;
	}
	if(key == ZKEY_OFF)
		return 0x02;

	key--;
	z1->key.matrix[key / 0x10] |= (1 << (key % 0x10));
	return 0x0c;
}

/*
	キーを離す
*/
int releaseKey(Z1stat *z1, uint8 key)
{
	if(key == 0)
		return -1;
	if(key == ZKEY_DEBUG)
		return -1;
	if(key == ZKEY_REWIND_INPORT)
		return -1;
	if(key == ZKEY_REWIND_OUTPORT)
		return -1;
	if(key == ZKEY_OFF)
		return -1;

	key--;
	z1->key.matrix[key / 0x10] &= ~(1 << (key % 0x10));
	return 0x0c;
}

/*
	自動入力する
*/
int autoUpdateKey(Z1stat *z1)
{
	struct AutoKey *auto_key = &z1->auto_key;
	int intr;

	if(auto_key->key == NULL)
		return -1;
	if(*auto_key->key == 0) {
		auto_key->count = 0;
		free(auto_key->key);
		auto_key->key = NULL;
		return -1;
	}

	if(auto_key->count-- > 0)
		return 0;
	if(*auto_key->key == ZKEY_CAPS || *auto_key->key == ZKEY_2ND)
		auto_key->count = z1->setting.refresh_rate / 20;
	else
		auto_key->count = z1->setting.refresh_rate / 10;

	if(auto_key->press) {
		intr = releaseKey(z1, *auto_key->key);
		memmove(auto_key->key, auto_key->key + 1, strlen((const char *)auto_key->key + 1) + 1);
	} else
		intr = pressKey(z1, *auto_key->key);
	auto_key->press = !auto_key->press;
	return intr;
}

/*
	割込が発生するか?
*/
static int checkIntr(Z1stat *z1, const uint16 *control)
{
	/* 割込禁止か? */
	if(*control & 0x08)
		return FALSE;

	/* 割込が発生しているか? */
	if(control == &z1->timer.control && z1->timer.intr)
		return FALSE;
	if(control == &z1->sio.control && z1->sio.intr)
		return FALSE;
	if(control == &z1->key.control && z1->key.intr)
		return FALSE;
	if(control == &z1->sw.control && z1->sw.intr)
		return FALSE;

	/* 他の優先度の高い割込が発生しているか? */ /* ??? */
/*
	if(control != &z1->timer.control && (*control & 0x03) >= (z1->timer.control & 0x03) && z1->timer.intr)
		return FALSE;
	if(control != &z1->sio.control && (*control & 0x03) >= (z1->sio.control & 0x03) && z1->sio.intr)
		return FALSE;
	if(control != &z1->key.control && (*control & 0x03) >= (z1->key.control & 0x03) && z1->key.intr)
		return FALSE;
	if(control != &z1->sw.control && (*control & 0x03) >= (z1->sw.control & 0x03) && z1->sw.intr)
		return FALSE;
*/
	return TRUE;
}

/*
	割込を開始する
*/
static void startIntr(Z1stat *z1, int mask)
{
	/* タイマ */
	if(mask == 0x0001) {
		if(z1->timer.intr == 0)
			z1->timer.intr = 1;
	} else {
		if(z1->timer.intr > 0)
			z1->timer.intr++;
	}

	/* シリアル */
	if(mask == 0x0004) {
		if(z1->sio.intr == 0)
			z1->sio.intr = 1;
	} else {
		if(z1->sio.intr > 0)
			z1->sio.intr++;
	}

	/* キー */
	if(mask == 0x0010) {
		if(z1->key.intr == 0)
			z1->key.intr = 1;
	} else {
		if(z1->key.intr > 0)
			z1->key.intr++;
	}

	/* 電源スイッチ */
	if(mask == 0x0020) {
		if(z1->sw.intr == 0)
			z1->sw.intr = 1;
	} else {
		if(z1->sw.intr > 0)
			z1->sw.intr++;
	}
}

/*
	実行するステート数を得る
*/
static int getExecStates(Z1stat *z1, int *states)
{
	int rest, r0, r1, r2, v = -1;

	/* 残り時間が最も短いタイマを得る */
	r0 = getRestCount(z1, &z1->timer.t0) - 1;
	r1 = getRestCount(z1, &z1->timer.t1) - 1;
	r2 = getRestCount(z1, &z1->timer.t2) - 1;
	if(r0 < r1)
		rest = r0 < r2 ? r0: r2;
	else
		rest = r1 < r2 ? r1: r2;
	if(rest <= 0)
		rest = 1;

	/* 実行するステート数を得る */
	*states = rest * 4;

	/* タイマを進める */
	if(foreCount(z1, &z1->timer.t0, rest))
		v = 0x08;
	if(foreCount(z1, &z1->timer.t1, rest))
		v = 0x12;
	if(foreCount(z1, &z1->timer.t2, rest))
		if(v < 0)
			v = 0x13;
	return v;
}

int main(int argc, char *argv[])
{
	Z1stat *z1 = malloc(sizeof(Z1stat));
	int start = -1, states = 0, s, v, i;

#if SDL_MAJOR_VERSION == 1
#ifdef _WIN32
	argv = argvToUTF8(argc, argv);
#endif
#endif

	/* エミュレータを初期化する */
	if(!init(z1, argc, argv))
		return 1;

#if 0
	writeWave("sound.wav", NULL, 0);
#endif

	/* RAMを読み込む */
	memset(z1->memory, 0, sizeof(z1->memory));
	if(readBin(z1->setting.path_ram, 0x40000, z1->memory) <= 0)
		for(i = 0; i < 0x20000; i++)
			z1->memory[i] = i & 0xff;

	for(i = 1; i < argc; i++)
		if(*argv[i] != '-') {
			if(readZ1Bin(argv[i], z1->memory, NULL, &start) <= 0) {
				showError("CANNOT OPEN FILE. (%s)\n", argv[i]);
				return 1;
			} else {
				for(i = i + 1; i < argc; i++)
					if(*argv[i] != '-')
						start = atoix(argv[i]);
			}
		}

	/* ROMを読み込む */
	if(strcmp(z1->setting.path_rom, "") == 0) {
		if(start < 0) {
			showError("NO ROM IMAGE.\n");
			return 1;
		}
		z1->cpu.i.bios = TRUE;
	} else if(readROM(z1->setting.path_rom, z1->memory + 0x40000) <= 0) {
		showError("CANNOT OPEN ROM IMAGE FILE. (%s)\n", z1->setting.path_rom);
		if(start < 0)
			return 1;
		z1->cpu.i.bios = TRUE;
	} else
		z1->cpu.i.bios = FALSE;

	if(z1->cpu.i.bios) {
		memcpy(z1->memory + 0x40000, pseudoROM, sizeof(pseudoROM));
		memset(z1->memory + 0x50000, 0xcf, 1);
	}

	/* リセットする */
	i86reset(&z1->cpu);

	if(z1->cpu.i.bios) {
		z1->cpu.r16.cs = 0;
		z1->cpu.r16.ip = start;
		start = -1;
	}

	for(;;) {
		/* 1周期分待つ */
		do {
			states += z1->setting.cpu_clock / z1->setting.refresh_rate;
			if(z1->auto_key.key == NULL)
				delay(1000 / z1->setting.refresh_rate);
		} while(states < 0);

		do {
			/* 実行するステート数を得る */
			if((v = getExecStates(z1, &s)) >= 0)
				if((z1->cpu.r16.f & 0x0200) && checkIntr(z1, &z1->timer.control)) {
					startIntr(z1, 0x0001);
					i86int(&z1->cpu, v);
				}

			/* 実行する */
			z1->cpu.i.states = s;
			if(i86exec(&z1->cpu) == I86_HALT) {
				if((z1->power & 0x0001) && !(z1->cpu.r16.f & 0x0200))
					goto last;

				if(start >= 0) {
					z1->cpu.r16.cs = 0;
					z1->cpu.r16.ip = start;
					z1->cpu.r16.hlt = 0;
					start = -1;
				}
			}

			/* タイマカウンタを再設定する */
			z1->timer.t0.count = getTimerCount(z1, &z1->timer.t0);
			z1->timer.t1.count = getTimerCount(z1, &z1->timer.t1);
			z1->timer.t2.count = getTimerCount(z1, &z1->timer.t2);

			/* シリアルポートに送信したら割込を発生させる */
			if(z1->sio.sent) {
				if((z1->cpu.r16.f & 0x0200) && checkIntr(z1, &z1->sio.control)) {
					startIntr(z1, 0x0004);
					i86int(&z1->cpu, 0x15);
				}
				z1->sio.sent = FALSE;
			}

			/* 残り実行ステート数を減らす */
			states -= (s - z1->cpu.i.states);
		} while(states > 0);

		/* サウンドバッファを切り替える */
		flipSoundBuffer(z1);

		/* 画面を更新する */
		updateLCD(z1);

		/* キーを更新する */
		switch((v = updateKey(z1))) {
		case 0x0c: /* キー割込 */
			if((z1->cpu.r16.f & 0x0200) && checkIntr(z1, &z1->key.control)) {
				if((z1->key.key_control & 0x02) && !z1->key.key_intr) {
					startIntr(z1, 0x0010);
					z1->key.key_intr = TRUE;
					i86int(&z1->cpu, v);
				}
			}
			break;
		case 0x02: /* ??? */
			if((z1->cpu.r16.f & 0x0200) && checkIntr(z1, &z1->sw.control)) {
				startIntr(z1, 0x0020);
				i86int(&z1->cpu, v);
			}
			break;
		}

		/* シリアルポートから受信する */
		if((z1->cpu.r16.f & 0x0200) && checkIntr(z1, &z1->sio.control)) {
			if(peekIOData(&z1->rs_receive, NULL)) {
				startIntr(z1, 0x0004);
				i86int(&z1->cpu, 0x14);
			}
		}
	}
last:;

	/* RAMの内容を保存する */
	if(strcmp(z1->setting.path_ram, "") != 0)
		writeBin(z1->setting.path_ram, z1->memory, 0x40000);
	return 0;
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
