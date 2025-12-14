/*
	設定ファイル処理(conf.c)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include "conf.h"
#ifdef _WIN32
#include "windows.h"
#endif

#define FALSE	0
#define TRUE	1

/* 設定ファイルにUTF-8のBOMがあるか? */
static int isUtf8;

/* コメント */
#define COMMENT	'#'

/* Y/N */
const static OptTable tableYesNo[] = {
	{ "y", TRUE },
	{ "1", TRUE },
	{ "n", FALSE },
	{ "0", FALSE },
	{ NULL, 0 }
};

/* ホームディレクトリ (win32のみ) */
#ifdef _WIN32
static char homeDir[PATH_MAX] = "";
#endif

/* 汎用バッファ (win32のみ) */
#ifdef _WIN32
static char _buffer[PATH_MAX];
#endif

/*
	引数の文字列をUTF-8に変換する (win32専用)
*/
#ifdef _WIN32
char **argvToUTF8(int argc, char *argv[])
{
	int i, len;
	wchar_t *utf16 = malloc(8);
	char **argv_utf8;

	argv_utf8 = malloc(sizeof(char *) * (argc + 1));

	for(i = 0; i < argc; i++) {
		len = (MultiByteToWideChar(CP_ACP, 0, (LPCSTR )argv[i], -1, NULL, 0) + 1) * sizeof(wchar_t);
		utf16 = realloc(utf16, len);
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR )argv[i], -1, (LPWSTR )utf16, len);

		len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR )utf16, -1, NULL, 0, NULL, NULL) + 1;
		argv_utf8[i] = malloc(len);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR )utf16, -1, (LPSTR )argv_utf8[i], len, NULL, NULL);
	}
	argv_utf8[i] = NULL;

	free(utf16);
	return argv_utf8;
}
#endif

/*
	実行ファイルのディレクトリを得る (win32専用) (下請け)
*/
#ifdef _WIN32
static char *getexedir(void)
{
	wchar_t w_module_name[PATH_MAX];
	char *p;

	GetModuleFileNameW(NULL, w_module_name, sizeof(w_module_name));
	WideCharToMultiByte(CP_UTF8, 0, w_module_name, -1, _buffer, sizeof(_buffer), NULL, NULL);

	for(p = _buffer + strlen(_buffer); p > _buffer && *p != '\\'; p--)
		;
	*p = 0;

	return _buffer;
}
#else
#define getexedir()	NULL
#endif

/*
	環境変数の値を得る (win32専用) (下請け)
*/
#if defined(_WIN32)
static const char *utf8_getenv(const char *varname)
{
	wchar_t w_varname[256], *value;

	MultiByteToWideChar(CP_UTF8, 0, varname, -1, w_varname, sizeof(w_varname));
	if((value = _wgetenv(w_varname)) == NULL) {
		strcpy(_buffer, "");
		return NULL;
	}
	WideCharToMultiByte(CP_UTF8, 0, value, -1, _buffer, sizeof(_buffer), NULL, NULL);
	return _buffer;
}
#define getenv(varname)	utf8_getenv(varname)
#endif

/*
	実行ファイルの引数からオプションを得る (下請け)
*/
static int readArg(Conf *conf, char *argv)
{
	int size;
	char *p, *q;

	if(*argv != '-')
		return 0;

	for(p = argv; *p != '=' && *p != '\0'; p++)
		;
	if(*p == '\0')
		return 0;

	for(q = p; *q != '\0'; q++)
		;
	
	size = (int )(p - argv - 1);
	memcpy(conf->key, argv + 1, size);
	*(conf->key + size) = '\0';

	size = (int )(q - p + 1);
	memcpy(conf->value, p + 1, size);
	*(conf->value + size) = '\0';

	*(conf + 1)->key = '\0';
	return 1;
}

/*
	Configファイルをオープンする (下請け)
*/
static FILE *openConfig(const char *file)
{
	const static char *hide[] = { ".", "" };
	FILE *fp;
	char path[FILENAME_MAX], *mode = "r";
	const char *dir[2], **p, **q;
#ifdef _WIN32
	wchar_t wpath[PATH_MAX], wmode[4];
#endif

	isUtf8 = FALSE;

#ifdef _WIN32
	dir[0] = getenv("USERPROFILE");
	dir[1] = getexedir();
#else
	dir[0] = getenv("HOME");
	dir[1] = NULL;
#endif

	for(p = hide; p != hide + sizeof(hide) / sizeof(hide[0]); p++)
		for(q = dir; q != dir + sizeof(dir) / sizeof(dir[0]); q++) {
			if(*q == NULL)
				continue;

#ifdef _WIN32
			strcpy(homeDir, *q);
#endif

			sprintf(path, "%s/%s%s", *q, *p, file);
#ifdef _WIN32
			MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, sizeof(wpath));
			MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, sizeof(wmode));
			fp = _wfopen(wpath, wmode);
#else
			fp = fopen(path, mode);
#endif
			if(fp != NULL)
				return fp;
		}
	return NULL;
}

/*
	ファイルからオプションを得る (下請け)
*/
static int readConfig(FILE *fp, Conf *conf)
{
	char buf[160], *p, *q;

	/* 1行読み込む */
	fgets(buf, sizeof(buf), fp);
	if(memcmp(buf, "\xef\xbb\xbf", 3) == 0) {
		isUtf8 = TRUE;
		memmove(buf, buf + 3, strlen(buf + 3) + 1);
	}

	/* 左辺を得る */
	for(p = buf; *p == ' ' || *p == '\t'; p++)
		;
	if(*p == COMMENT || *p == '\r' || *p == '\n' || *p == 0)
		return 0;
	for(q = p; *q != ' ' && *q != '\t' && *q != '\r' && *q != '\n' && *q != 0; q++)
		;
	memcpy(conf->key, p, (int )(q - p));
	*(conf->key + (int )(q - p)) = '\0';
	
	/* 右辺を得る */
	for(p = q; *p == ' ' || *p == '\t'; p++)
		;
	for(q = p; *q != '\r' && *q != '\n' && *q != 0 && *q != COMMENT; q++)
		;
	if(p < q)
		for(; *(q - 1) == ' ' || *(q - 1) == '\t'; q--)
			;
	*q++ = 0;

	/* 値を書き込む */
#ifdef _WIN32
	if(isUtf8)
		strcpy(conf->value, p);
	else {
		wchar_t wbuf[160];
		MultiByteToWideChar(CP_ACP, 0, p, -1, wbuf, sizeof(wbuf));
		WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, conf->value, sizeof(conf->value), NULL, NULL);
	}
#else
	strcpy(conf->value, p);
#endif

	/* 終端を書き込む */
	*(conf + 1)->key = '\0';
	return 1;
}

/*
	Configファイルを読み込む 
*/
Conf *getConfig(Conf *conf, int length, const char *file, int argc, char *argv[])
{
	Conf *p = conf, *last = conf + length - 1;
	FILE *fp;
	int i, line;

	strcpy(p->key, "");
	
	/* 実行ファイルの引数からオプションを得る */
	line = INT_MIN;
	for(i = 1; i < argc && p < last; i++) {
		if(readArg(p, argv[i]))
			(p++)->line = line;
		line++;
	}

	/* ファイルからオプションを得る */
	if((fp = openConfig(file)) != NULL) {
		line = 1;
		while(!feof(fp) && p < last) {
			if(readConfig(fp, p))
				(p++)->line = line;
			line++;
		}
		fclose(fp);
	}

	return (p != conf ? conf: NULL);
}

/*
	Configファイルから文字列を取り出す
*/
const char *getOptText(const Conf *conf, const char *key, const char *default_value)
{
	const Conf *p;

	for(p = conf; p->key[0] != '\0'; p++)
		if(strcasecmp(p->key, key) == 0)
			return p->value;
	return default_value;
}

/*
	Configファイルから数値を取り出す(10進数)
*/
int getOptInt(const Conf *conf, const char *key, int default_value)
{
	const char *p;

	p = getOptText(conf, key, "");
	if(strcmp(p, "") != 0)
		return atoi(p);
	else
		return default_value;
}

/*
	Configファイルから数値を取り出す(16進数)
*/
unsigned int getOptHex(const Conf *conf, const char *key, unsigned int default_value)
{
	int x;
	const char *p;

	p = getOptText(conf, key, "");
	if(strcmp(p, "") != 0) {
		sscanf(p, "%x", &x);
		return x;
	} else
		return default_value;
}

/*
	テーブルからキーが一致する値を得る(getOptTableの下請け) (下請け)
*/
static int textToInt(const OptTable *table, const char *str, int default_value)
{
	const OptTable *p;
	int value = default_value;

	for(p = table; p->string != NULL; p++)
		if(strcasecmp(str, p->string) == 0)
			value = p->value;
	return value;
}

/*
	Configファイルから文字列を取り出し, テーブルを利用して数値に変換する
*/
int getOptTable(const Conf *conf, const char *key, const OptTable *table, int default_value)
{
	return textToInt(table, getOptText(conf, key, ""), default_value);
}

/*
	ConfigファイルからY/Nを得る
*/
int getOptYesNo(const Conf *conf, const char *key, int default_value)
{
	return getOptTable(conf, key, tableYesNo, default_value);
}

/*
	テーブルに列を挿入する
*/
void insOptTable(OptTable *table, const char *key, int value)
{
	OptTable *p;

	for(p = table; p->string != NULL; p++)
		;
	p->string = malloc(strlen(key) + 1);
	strcpy(p->string, key);
	p->value = value;
	p++;
	p->string = NULL;
	p->value = 0;
}

/*
	~をホームディレクトリに置き換える
*/
char *setHomeDir(char *buf, const char *path)
{
	if(*path != '~') {
		strcpy(buf, path);
		return buf;
	}

#ifdef _WIN32
	if(strcmp(homeDir, "") != 0)
		sprintf(buf, "%s%s", homeDir, path + 1);
	else
#endif
	if(getenv("HOME") != NULL)
		sprintf(buf, "%s%s", getenv("HOME"), path + 1);
	else if(getenv("USERPROFILE") != NULL)
		sprintf(buf, "%s%s", getenv("USERPROFILE"), path + 1);
	else if(getexedir() != NULL)
		sprintf(buf, "%s%s", getexedir(), path + 1);
	else
		sprintf(buf, "%s%s", ".", path + 1);
	return buf;
}

/*
	Copyright 2005 ~ 2019 maruhiro
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
