/*
	CASIO Z-1/FX-890P emulator
	初期化
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "z1.h"

/* マシン */
const static OptTable tableMachine[] = {
	{ "z1", MACHINE_Z1 },
	{ "z1gr", MACHINE_Z1GR },
	{ "z1gra", MACHINE_Z1GRA },
	{ "fx890p", MACHINE_FX890P },
	{ "fx890p_en", MACHINE_FX890P_EN },
	{ NULL, 0 }
};

/* Z-1/FX-890Pのキー */
const static OptTable tableZkey[] = {
	{ "brk", ZKEY_BRK },
	{ "tab", ZKEY_TAB },
	{ "q", ZKEY_Q },
	{ "allreset", ZKEY_ALLRESET },
	{ "a", ZKEY_A },
	{ "caps", ZKEY_CAPS },
	{ "z", ZKEY_Z },
	{ "w", ZKEY_W },
	{ "e", ZKEY_E },
	{ "s", ZKEY_S },
	{ "d", ZKEY_D },
	{ "x", ZKEY_X },
	{ "c", ZKEY_C },
	{ "srch", ZKEY_SRCH },
	{ "in", ZKEY_IN },
	{ "r", ZKEY_R },
	{ "t", ZKEY_T },
	{ "f", ZKEY_F },
	{ "g", ZKEY_G },
	{ "v", ZKEY_V },
	{ "b", ZKEY_B },
	{ "out", ZKEY_OUT },
	{ "calc", ZKEY_CALC },
	{ "y", ZKEY_Y },
	{ "u", ZKEY_U },
	{ "h", ZKEY_H },
	{ "j", ZKEY_J },
	{ "n", ZKEY_N },
	{ "m", ZKEY_M },
	{ "spc", ZKEY_SPC },
	{ "=", ZKEY_EQUAL },
	{ "i", ZKEY_I },
	{ "o", ZKEY_O },
	{ "k", ZKEY_K },
	{ "l", ZKEY_L },
	{ ",", ZKEY_COMMA },
	{ "ins", ZKEY_INS },
	{ "left", ZKEY_LEFT },
	{ "down", ZKEY_DOWN },
	{ "p", ZKEY_P },
	{ "2nd", ZKEY_2ND },
	{ ";", ZKEY_SEMICOLON },
	{ ":", ZKEY_COLON },
	{ "up", ZKEY_UP },
	{ "del", ZKEY_DEL },
	{ "right", ZKEY_RIGHT },
	{ "0", ZKEY_0 },
	{ "menu", ZKEY_MENU },
	{ "log", ZKEY_LOG },
	{ "mr", ZKEY_MR },
	{ "7", ZKEY_7 },
	{ "4", ZKEY_4 },
	{ "1", ZKEY_1 },
	{ ".", ZKEY_PERIOD },
	{ "exponent", ZKEY_EXP },
	{ "return", ZKEY_RETURN },
	{ "cal", ZKEY_CAL },
	{ "ln", ZKEY_LN },
	{ "m+", ZKEY_MPLUS },
	{ "8", ZKEY_8 },
	{ "5", ZKEY_5 },
	{ "2", ZKEY_2 },
	{ "3", ZKEY_3 },
	{ "+", ZKEY_PLUS },
	{ "-", ZKEY_MINUS },
	{ "degr", ZKEY_DEGR },
	{ "sin", ZKEY_SIN },
	{ "(", ZKEY_LPAREN },
	{ ")", ZKEY_RPAREN },
	{ "9", ZKEY_9 },
	{ "6", ZKEY_6 },
	{ "*", ZKEY_ASTER },
	{ "/", ZKEY_SLASH },
	{ "bs", ZKEY_BS },
	{ "sqr", ZKEY_SQR },
	{ "x^2", ZKEY_X2 },
	{ "eng", ZKEY_ENG },
	{ "cls", ZKEY_CLS },
	{ "cos", ZKEY_COS },
	{ "^", ZKEY_HAT },
	{ "ans", ZKEY_ANS },
	{ "tan", ZKEY_TAN },
	{ "off", ZKEY_OFF }, /* 仮想キー */
	{ "copy", ZKEY_COPY }, /* 仮想キー */
	{ "paste", ZKEY_PASTE }, /* 仮想キー */
	{ "rewind_inport", ZKEY_REWIND_INPORT }, /* 仮想キー */
	{ "rewind_outport", ZKEY_REWIND_OUTPORT }, /* 仮想キー */
	{ "trace", ZKEY_DEBUG }, /* 仮想キー */
	{ "!", ZKEYMOD_SHIFT | ZKEY_Q },
	{ "@", ZKEYMOD_SHIFT | ZKEY_A },
	{ "kana", ZKEYMOD_SHIFT | ZKEY_CAPS },
	{ "print", ZKEYMOD_SHIFT | ZKEY_Z },
	{ "\"", ZKEYMOD_SHIFT | ZKEY_W },
	{ "hash", ZKEYMOD_SHIFT| ZKEY_E },
	{ "~", ZKEYMOD_SHIFT | ZKEY_S },
	{ "?", ZKEYMOD_SHIFT | ZKEY_D },
	{ "system", ZKEYMOD_SHIFT | ZKEY_X },
	{ "clear", ZKEYMOD_SHIFT | ZKEY_C },
	{ "line", ZKEYMOD_SHIFT | ZKEY_SRCH },
	{ "$", ZKEYMOD_SHIFT | ZKEY_R },
	{ "%", ZKEYMOD_SHIFT | ZKEY_T },
	{ "{", ZKEYMOD_SHIFT | ZKEY_F },
	{ "}", ZKEYMOD_SHIFT | ZKEY_G },
	{ "cont", ZKEYMOD_SHIFT | ZKEY_V },
	{ "renum", ZKEYMOD_SHIFT | ZKEY_B },
	{ "&", ZKEYMOD_SHIFT | ZKEY_Y },
	{ "'", ZKEYMOD_SHIFT | ZKEY_U },
	{ "[", ZKEYMOD_SHIFT | ZKEY_H },
	{ "]", ZKEYMOD_SHIFT | ZKEY_J },
	{ "run", ZKEYMOD_SHIFT | ZKEY_N },
	{ "edit", ZKEYMOD_SHIFT | ZKEY_M },
	{ "delete", ZKEYMOD_SHIFT | ZKEY_EQUAL },
	{ "\\", ZKEYMOD_SHIFT | ZKEY_I },
	{ "|", ZKEYMOD_SHIFT | ZKEY_O },
	{ "<", ZKEYMOD_SHIFT | ZKEY_K },
	{ ">", ZKEYMOD_SHIFT | ZKEY_L },
	{ "list", ZKEYMOD_SHIFT | ZKEY_COMMA },
	{ "ltop", ZKEYMOD_SHIFT | ZKEY_LEFT },
	{ "fend", ZKEYMOD_SHIFT | ZKEY_DOWN },
	{ "`", ZKEYMOD_SHIFT | ZKEY_P },
	{ "_", ZKEYMOD_SHIFT | ZKEY_SEMICOLON },
	{ "angle", ZKEYMOD_SHIFT | ZKEY_COLON },
	{ "ftop", ZKEYMOD_SHIFT | ZKEY_UP },
	{ "lend", ZKEYMOD_SHIFT | ZKEY_RIGHT },
	{ "p0", ZKEYMOD_SHIFT | ZKEY_0 },
	{ "submenu", ZKEYMOD_SHIFT | KEY_MENU },
	{ "10^", ZKEYMOD_SHIFT | ZKEY_LOG },
	{ "min", ZKEYMOD_SHIFT | ZKEY_MR },
	{ "p7", ZKEYMOD_SHIFT | ZKEY_7 },
	{ "p4", ZKEYMOD_SHIFT | ZKEY_4 },
	{ "p1", ZKEYMOD_SHIFT | ZKEY_1 },
	{ "ran", ZKEYMOD_SHIFT | ZKEY_PERIOD },
	{ "pi", ZKEYMOD_SHIFT | ZKEY_EXP },
	{ "exp", ZKEYMOD_SHIFT | ZKEY_LN },
	{ "m-", ZKEYMOD_SHIFT | ZKEY_MPLUS },
	{ "p8", ZKEYMOD_SHIFT | ZKEY_8 },
	{ "p5", ZKEYMOD_SHIFT | ZKEY_5 },
	{ "p2", ZKEYMOD_SHIFT | ZKEY_2 },
	{ "p3", ZKEYMOD_SHIFT | ZKEY_3 },
	{ "pol", ZKEYMOD_SHIFT | ZKEY_PLUS },
	{ "rec", ZKEYMOD_SHIFT | ZKEY_MINUS },
	{ "dms", ZKEYMOD_SHIFT | ZKEY_DEGR },
	{ "asn", ZKEYMOD_SHIFT | ZKEY_SIN },
	{ "&h", ZKEYMOD_SHIFT | ZKEY_LPAREN },
	{ "hex", ZKEYMOD_SHIFT | ZKEY_RPAREN },
	{ "p9", ZKEYMOD_SHIFT | ZKEY_9 },
	{ "p6", ZKEYMOD_SHIFT | ZKEY_6 },
	{ "npr", ZKEYMOD_SHIFT | ZKEY_ASTER },
	{ "ncr", ZKEYMOD_SHIFT | ZKEY_SLASH },
	{ "lcan", ZKEYMOD_SHIFT | ZKEY_BS },
	{ "cur", ZKEYMOD_SHIFT | ZKEY_SQR },
	{ "x^3", ZKEYMOD_SHIFT | ZKEY_X2 },
	{ "seng", ZKEYMOD_SHIFT | ZKEY_ENG },
	{ "home", ZKEYMOD_SHIFT | ZKEY_CLS },
	{ "acs", ZKEYMOD_SHIFT | ZKEY_COS },
	{ "fact", ZKEYMOD_SHIFT | ZKEY_HAT },
	{ "set", ZKEYMOD_SHIFT | ZKEY_ANS },
	{ "atn", ZKEYMOD_SHIFT | ZKEY_TAN },
	{ NULL, 0 }
};

/* 対応付けるキー */
static OptTable tableKey[512] = {
	{ "none", 0 },
	{ "backspace", KEY_BACKSPACE },
	{ "tab", KEY_TAB },
	{ "clear", KEY_CLEAR },
	{ "return", KEY_RETURN },
	{ "enter", KEY_RETURN },
	{ "pause", KEY_PAUSE },
	{ "escape", KEY_ESCAPE },
	{ "space", KEY_SPACE },
	{ ":", KEY_COLON },
	{ ",", KEY_COMMA },
	{ "-", KEY_MINUS },
	{ ".", KEY_PERIOD },
	{ "/", KEY_SLASH },
	{ "0", KEY_0 },
	{ "1", KEY_1 },
	{ "2", KEY_2 },
	{ "3", KEY_3 },
	{ "4", KEY_4 },
	{ "5", KEY_5 },
	{ "6", KEY_6 },
	{ "7", KEY_7 },
	{ "8", KEY_8 },
	{ "9", KEY_9 },
	{ ";", KEY_SEMICOLON },
	{ "^", KEY_HAT },
	{ "@", KEY_AT },
	{ "]", KEY_RIGHTBRACKET },
	{ "[", KEY_LEFTBRACKET },
	{ "`", KEY_BACKQUOTE },
	{ "a", KEY_A },
	{ "b", KEY_B },
	{ "c", KEY_C },
	{ "d", KEY_D },
	{ "e", KEY_E },
	{ "f", KEY_F },
	{ "g", KEY_G },
	{ "h", KEY_H },
	{ "i", KEY_I },
	{ "j", KEY_J },
	{ "k", KEY_K },
	{ "l", KEY_L },
	{ "m", KEY_M },
	{ "n", KEY_N },
	{ "o", KEY_O },
	{ "p", KEY_P },
	{ "q", KEY_Q },
	{ "r", KEY_R },
	{ "s", KEY_S },
	{ "t", KEY_T },
	{ "u", KEY_U },
	{ "v", KEY_V },
	{ "w", KEY_W },
	{ "x", KEY_X },
	{ "y", KEY_Y },
	{ "z", KEY_Z },
	{ "delete", KEY_DELETE },
	{ "n0", KEY_KP0 },
	{ "n1", KEY_KP1 },
	{ "n2", KEY_KP2 },
	{ "n3", KEY_KP3 },
	{ "n4", KEY_KP4 },
	{ "n5", KEY_KP5 },
	{ "n6", KEY_KP6 },
	{ "n7", KEY_KP7 },
	{ "n8", KEY_KP8 },
	{ "n9", KEY_KP9 },
	{ "n.", KEY_KP_PERIOD },
	{ "n/", KEY_KP_DIVIDE },
	{ "n*", KEY_KP_MULTIPLY },
	{ "n-", KEY_KP_MINUS },
	{ "n+", KEY_KP_PLUS },
	{ "nreturn", KEY_KP_ENTER },
	{ "nenter", KEY_KP_ENTER },
	{ "n=", KEY_KP_EQUALS },
	{ "up", KEY_UP },
	{ "down", KEY_DOWN },
	{ "right", KEY_RIGHT },
	{ "left", KEY_LEFT },
	{ "insert", KEY_INSERT },
	{ "home", KEY_HOME },
	{ "end", KEY_END },
	{ "pageup", KEY_PAGEUP },
	{ "pagedown", KEY_PAGEDOWN  },
	{ "f1", KEY_F1 },
	{ "f2", KEY_F2 },
	{ "f3", KEY_F3 },
	{ "f4", KEY_F4 },
	{ "f5", KEY_F5 },
	{ "f6", KEY_F6 },
	{ "f7", KEY_F7 },
	{ "f8", KEY_F8 },
	{ "f9", KEY_F9 },
	{ "f10", KEY_F10 },
	{ "f11", KEY_F11 },
	{ "f12", KEY_F12 },
	{ "f13", KEY_F13 },
	{ "f14", KEY_F14 },
	{ "f15", KEY_F15 },
	{ "numlock", KEY_NUMLOCK },
	{ "capslock", KEY_CAPSLOCK },
	{ "scrolllock", KEY_SCROLLOCK },
	{ "rshift", KEY_RSHIFT },
	{ "lshift", KEY_LSHIFT },
	{ "rctrl", KEY_RCTRL },
	{ "lctrl", KEY_LCTRL },
	{ "ralt", KEY_RALT },
	{ "lalt", KEY_LALT },
	{ "mode", KEY_MODE },
	{ "compose", KEY_COMPOSE },
	{ "help", KEY_HELP },
	{ "print", KEY_PRINT },
	{ "sysreq", KEY_SYSREQ },
	{ "break", KEY_BREAK },
	{ "menu", KEY_MENU },
	{ "power", KEY_POWER },
	{ "_", KEY_UNDERSCORE },
	{ "kana", KEY_KANA },
	{ "\\", KEY_YEN  },
	{ "xfer", KEY_XFER },
	{ "nfer", KEY_NFER },
	{ "non-us-backslash", KEY_NONUSBACKSLASH },
	{ "%backspace", KEYMOD_ALT | KEY_BACKSPACE },
	{ "%tab", KEYMOD_ALT | KEY_TAB },
	{ "%clear", KEYMOD_ALT | KEY_CLEAR },
	{ "%return", KEYMOD_ALT | KEY_RETURN },
	{ "%enter", KEYMOD_ALT | KEY_RETURN },
	{ "%pause", KEYMOD_ALT | KEY_PAUSE },
	{ "%escape", KEYMOD_ALT | KEY_ESCAPE },
	{ "%space", KEYMOD_ALT | KEY_SPACE },
	{ "%:", KEYMOD_ALT | KEY_COLON },
	{ "%,", KEYMOD_ALT | KEY_COMMA },
	{ "%-", KEYMOD_ALT | KEY_MINUS },
	{ "%.", KEYMOD_ALT | KEY_PERIOD },
	{ "%/", KEYMOD_ALT | KEY_SLASH },
	{ "%0", KEYMOD_ALT | KEY_0 },
	{ "%1", KEYMOD_ALT | KEY_1 },
	{ "%2", KEYMOD_ALT | KEY_2 },
	{ "%3", KEYMOD_ALT | KEY_3 },
	{ "%4", KEYMOD_ALT | KEY_4 },
	{ "%5", KEYMOD_ALT | KEY_5 },
	{ "%6", KEYMOD_ALT | KEY_6 },
	{ "%7", KEYMOD_ALT | KEY_7 },
	{ "%8", KEYMOD_ALT | KEY_8 },
	{ "%9", KEYMOD_ALT | KEY_9 },
	{ "%;", KEYMOD_ALT | KEY_SEMICOLON },
	{ "%^", KEYMOD_ALT | KEY_HAT },
	{ "%@", KEYMOD_ALT | KEY_AT },
	{ "%]", KEYMOD_ALT | KEY_RIGHTBRACKET },
	{ "%[", KEYMOD_ALT | KEY_LEFTBRACKET },
	{ "%`", KEYMOD_ALT | KEY_BACKQUOTE },
	{ "%a", KEYMOD_ALT | KEY_A },
	{ "%b", KEYMOD_ALT | KEY_B },
	{ "%c", KEYMOD_ALT | KEY_C },
	{ "%d", KEYMOD_ALT | KEY_D },
	{ "%e", KEYMOD_ALT | KEY_E },
	{ "%f", KEYMOD_ALT | KEY_F },
	{ "%g", KEYMOD_ALT | KEY_G },
	{ "%h", KEYMOD_ALT | KEY_H },
	{ "%i", KEYMOD_ALT | KEY_I },
	{ "%j", KEYMOD_ALT | KEY_J },
	{ "%k", KEYMOD_ALT | KEY_K },
	{ "%l", KEYMOD_ALT | KEY_L },
	{ "%m", KEYMOD_ALT | KEY_M },
	{ "%n", KEYMOD_ALT | KEY_N },
	{ "%o", KEYMOD_ALT | KEY_O },
	{ "%p", KEYMOD_ALT | KEY_P },
	{ "%q", KEYMOD_ALT | KEY_Q },
	{ "%r", KEYMOD_ALT | KEY_R },
	{ "%s", KEYMOD_ALT | KEY_S },
	{ "%t", KEYMOD_ALT | KEY_T },
	{ "%u", KEYMOD_ALT | KEY_U },
	{ "%v", KEYMOD_ALT | KEY_V },
	{ "%w", KEYMOD_ALT | KEY_W },
	{ "%x", KEYMOD_ALT | KEY_X },
	{ "%y", KEYMOD_ALT | KEY_Y },
	{ "%z", KEYMOD_ALT | KEY_Z },
	{ "%delete", KEYMOD_ALT | KEY_DELETE },
	{ "%n0", KEYMOD_ALT | KEY_KP0 },
	{ "%n1", KEYMOD_ALT | KEY_KP1 },
	{ "%n2", KEYMOD_ALT | KEY_KP2 },
	{ "%n3", KEYMOD_ALT | KEY_KP3 },
	{ "%n4", KEYMOD_ALT | KEY_KP4 },
	{ "%n5", KEYMOD_ALT | KEY_KP5 },
	{ "%n6", KEYMOD_ALT | KEY_KP6 },
	{ "%n7", KEYMOD_ALT | KEY_KP7 },
	{ "%n8", KEYMOD_ALT | KEY_KP8 },
	{ "%n9", KEYMOD_ALT | KEY_KP9 },
	{ "%n.", KEYMOD_ALT | KEY_KP_PERIOD },
	{ "%n/", KEYMOD_ALT | KEY_KP_DIVIDE },
	{ "%n*", KEYMOD_ALT | KEY_KP_MULTIPLY },
	{ "%n-", KEYMOD_ALT | KEY_KP_MINUS },
	{ "%n+", KEYMOD_ALT | KEY_KP_PLUS },
	{ "%nreturn", KEYMOD_ALT | KEY_KP_ENTER },
	{ "%nenter", KEYMOD_ALT | KEY_KP_ENTER },
	{ "%n=", KEYMOD_ALT | KEY_KP_EQUALS },
	{ "%up", KEYMOD_ALT | KEY_UP },
	{ "%down", KEYMOD_ALT | KEY_DOWN },
	{ "%right", KEYMOD_ALT | KEY_RIGHT },
	{ "%left", KEYMOD_ALT | KEY_LEFT },
	{ "%insert", KEYMOD_ALT | KEY_INSERT },
	{ "%home", KEYMOD_ALT | KEY_HOME },
	{ "%end", KEYMOD_ALT | KEY_END },
	{ "%pageup", KEYMOD_ALT | KEY_PAGEUP },
	{ "%pagedown", KEYMOD_ALT | KEY_PAGEDOWN  },
	{ "%f1", KEYMOD_ALT | KEY_F1 },
	{ "%f2", KEYMOD_ALT | KEY_F2 },
	{ "%f3", KEYMOD_ALT | KEY_F3 },
	{ "%f4", KEYMOD_ALT | KEY_F4 },
	{ "%f5", KEYMOD_ALT | KEY_F5 },
	{ "%f6", KEYMOD_ALT | KEY_F6 },
	{ "%f7", KEYMOD_ALT | KEY_F7 },
	{ "%f8", KEYMOD_ALT | KEY_F8 },
	{ "%f9", KEYMOD_ALT | KEY_F9 },
	{ "%f10", KEYMOD_ALT | KEY_F10 },
	{ "%f11", KEYMOD_ALT | KEY_F11 },
	{ "%f12", KEYMOD_ALT | KEY_F12 },
	{ "%f13", KEYMOD_ALT | KEY_F13 },
	{ "%f14", KEYMOD_ALT | KEY_F14 },
	{ "%f15", KEYMOD_ALT | KEY_F15 },
	{ "%numlock", KEYMOD_ALT | KEY_NUMLOCK },
	{ "%capslock", KEYMOD_ALT | KEY_CAPSLOCK },
	{ "%scrolllock", KEYMOD_ALT | KEY_SCROLLOCK },
	{ "%mode", KEYMOD_ALT | KEY_MODE },
	{ "%compose", KEYMOD_ALT | KEY_COMPOSE },
	{ "%help", KEYMOD_ALT | KEY_HELP },
	{ "%print", KEYMOD_ALT | KEY_PRINT },
	{ "%sysreq", KEYMOD_ALT | KEY_SYSREQ },
	{ "%break", KEYMOD_ALT | KEY_BREAK },
	{ "%menu", KEYMOD_ALT | KEY_MENU },
	{ "%power", KEYMOD_ALT | KEY_POWER },
	{ "%_", KEYMOD_ALT | KEY_UNDERSCORE },
	{ "%kana", KEYMOD_ALT | KEY_KANA },
	{ "%\\", KEYMOD_ALT | KEY_YEN  },
	{ "%xfer", KEYMOD_ALT | KEY_XFER },
	{ "%nfer", KEYMOD_ALT | KEY_NFER },
	{ "%non-us-backslash", KEYMOD_ALT | KEY_NONUSBACKSLASH },
	{ "^backspace", KEYMOD_CTRL | KEY_BACKSPACE },
	{ "^tab", KEYMOD_CTRL | KEY_TAB },
	{ "^clear", KEYMOD_CTRL | KEY_CLEAR },
	{ "^return", KEYMOD_CTRL | KEY_RETURN },
	{ "^enter", KEYMOD_CTRL | KEY_RETURN },
	{ "^pause", KEYMOD_CTRL | KEY_PAUSE },
	{ "^escape", KEYMOD_CTRL | KEY_ESCAPE },
	{ "^space", KEYMOD_CTRL | KEY_SPACE },
	{ "^:", KEYMOD_CTRL | KEY_COLON },
	{ "^,", KEYMOD_CTRL | KEY_COMMA },
	{ "^-", KEYMOD_CTRL | KEY_MINUS },
	{ "^.", KEYMOD_CTRL | KEY_PERIOD },
	{ "^/", KEYMOD_CTRL | KEY_SLASH },
	{ "^0", KEYMOD_CTRL | KEY_0 },
	{ "^1", KEYMOD_CTRL | KEY_1 },
	{ "^2", KEYMOD_CTRL | KEY_2 },
	{ "^3", KEYMOD_CTRL | KEY_3 },
	{ "^4", KEYMOD_CTRL | KEY_4 },
	{ "^5", KEYMOD_CTRL | KEY_5 },
	{ "^6", KEYMOD_CTRL | KEY_6 },
	{ "^7", KEYMOD_CTRL | KEY_7 },
	{ "^8", KEYMOD_CTRL | KEY_8 },
	{ "^9", KEYMOD_CTRL | KEY_9 },
	{ "^;", KEYMOD_CTRL | KEY_SEMICOLON },
	{ "^^", KEYMOD_CTRL | KEY_HAT },
	{ "^@", KEYMOD_CTRL | KEY_AT },
	{ "^]", KEYMOD_CTRL | KEY_RIGHTBRACKET },
	{ "^[", KEYMOD_CTRL | KEY_LEFTBRACKET },
	{ "^`", KEYMOD_CTRL | KEY_BACKQUOTE },
	{ "^a", KEYMOD_CTRL | KEY_A },
	{ "^b", KEYMOD_CTRL | KEY_B },
	{ "^c", KEYMOD_CTRL | KEY_C },
	{ "^d", KEYMOD_CTRL | KEY_D },
	{ "^e", KEYMOD_CTRL | KEY_E },
	{ "^f", KEYMOD_CTRL | KEY_F },
	{ "^g", KEYMOD_CTRL | KEY_G },
	{ "^h", KEYMOD_CTRL | KEY_H },
	{ "^i", KEYMOD_CTRL | KEY_I },
	{ "^j", KEYMOD_CTRL | KEY_J },
	{ "^k", KEYMOD_CTRL | KEY_K },
	{ "^l", KEYMOD_CTRL | KEY_L },
	{ "^m", KEYMOD_CTRL | KEY_M },
	{ "^n", KEYMOD_CTRL | KEY_N },
	{ "^o", KEYMOD_CTRL | KEY_O },
	{ "^p", KEYMOD_CTRL | KEY_P },
	{ "^q", KEYMOD_CTRL | KEY_Q },
	{ "^r", KEYMOD_CTRL | KEY_R },
	{ "^s", KEYMOD_CTRL | KEY_S },
	{ "^t", KEYMOD_CTRL | KEY_T },
	{ "^u", KEYMOD_CTRL | KEY_U },
	{ "^v", KEYMOD_CTRL | KEY_V },
	{ "^w", KEYMOD_CTRL | KEY_W },
	{ "^x", KEYMOD_CTRL | KEY_X },
	{ "^y", KEYMOD_CTRL | KEY_Y },
	{ "^z", KEYMOD_CTRL | KEY_Z },
	{ "^delete", KEYMOD_CTRL | KEY_DELETE },
	{ "^n0", KEYMOD_CTRL | KEY_KP0 },
	{ "^n1", KEYMOD_CTRL | KEY_KP1 },
	{ "^n2", KEYMOD_CTRL | KEY_KP2 },
	{ "^n3", KEYMOD_CTRL | KEY_KP3 },
	{ "^n4", KEYMOD_CTRL | KEY_KP4 },
	{ "^n5", KEYMOD_CTRL | KEY_KP5 },
	{ "^n6", KEYMOD_CTRL | KEY_KP6 },
	{ "^n7", KEYMOD_CTRL | KEY_KP7 },
	{ "^n8", KEYMOD_CTRL | KEY_KP8 },
	{ "^n9", KEYMOD_CTRL | KEY_KP9 },
	{ "^n.", KEYMOD_CTRL | KEY_KP_PERIOD },
	{ "^n/", KEYMOD_CTRL | KEY_KP_DIVIDE },
	{ "^n*", KEYMOD_CTRL | KEY_KP_MULTIPLY },
	{ "^n-", KEYMOD_CTRL | KEY_KP_MINUS },
	{ "^n+", KEYMOD_CTRL | KEY_KP_PLUS },
	{ "^nreturn", KEYMOD_CTRL | KEY_KP_ENTER },
	{ "^nenter", KEYMOD_CTRL | KEY_KP_ENTER },
	{ "^n=", KEYMOD_CTRL | KEY_KP_EQUALS },
	{ "^up", KEYMOD_CTRL | KEY_UP },
	{ "^down", KEYMOD_CTRL | KEY_DOWN },
	{ "^right", KEYMOD_CTRL | KEY_RIGHT },
	{ "^left", KEYMOD_CTRL | KEY_LEFT },
	{ "^insert", KEYMOD_CTRL | KEY_INSERT },
	{ "^home", KEYMOD_CTRL | KEY_HOME },
	{ "^end", KEYMOD_CTRL | KEY_END },
	{ "^pageup", KEYMOD_CTRL | KEY_PAGEUP },
	{ "^pagedown", KEYMOD_CTRL | KEY_PAGEDOWN  },
	{ "^f1", KEYMOD_CTRL | KEY_F1 },
	{ "^f2", KEYMOD_CTRL | KEY_F2 },
	{ "^f3", KEYMOD_CTRL | KEY_F3 },
	{ "^f4", KEYMOD_CTRL | KEY_F4 },
	{ "^f5", KEYMOD_CTRL | KEY_F5 },
	{ "^f6", KEYMOD_CTRL | KEY_F6 },
	{ "^f7", KEYMOD_CTRL | KEY_F7 },
	{ "^f8", KEYMOD_CTRL | KEY_F8 },
	{ "^f9", KEYMOD_CTRL | KEY_F9 },
	{ "^f10", KEYMOD_CTRL | KEY_F10 },
	{ "^f11", KEYMOD_CTRL | KEY_F11 },
	{ "^f12", KEYMOD_CTRL | KEY_F12 },
	{ "^f13", KEYMOD_CTRL | KEY_F13 },
	{ "^f14", KEYMOD_CTRL | KEY_F14 },
	{ "^f15", KEYMOD_CTRL | KEY_F15 },
	{ "^numlock", KEYMOD_CTRL | KEY_NUMLOCK },
	{ "^capslock", KEYMOD_CTRL | KEY_CAPSLOCK },
	{ "^scrolllock", KEYMOD_CTRL | KEY_SCROLLOCK },
	{ "^mode", KEYMOD_CTRL | KEY_MODE },
	{ "^compose", KEYMOD_CTRL | KEY_COMPOSE },
	{ "^help", KEYMOD_CTRL | KEY_HELP },
	{ "^print", KEYMOD_CTRL | KEY_PRINT },
	{ "^sysreq", KEYMOD_CTRL | KEY_SYSREQ },
	{ "^break", KEYMOD_CTRL | KEY_BREAK },
	{ "^menu", KEYMOD_CTRL | KEY_MENU },
	{ "^power", KEYMOD_CTRL | KEY_POWER },
	{ "^_", KEYMOD_CTRL | KEY_UNDERSCORE },
	{ "^kana", KEYMOD_CTRL | KEY_KANA },
	{ "^\\", KEYMOD_CTRL | KEY_YEN  },
	{ "^xfer", KEYMOD_CTRL | KEY_XFER },
	{ "^nfer", KEYMOD_CTRL | KEY_NFER },
	{ "^non-us-backslash", KEYMOD_CTRL | KEY_NONUSBACKSLASH },
	{ "+backspace", KEYMOD_SHIFT | KEY_BACKSPACE },
	{ "+tab", KEYMOD_SHIFT | KEY_TAB },
	{ "+clear", KEYMOD_SHIFT | KEY_CLEAR },
	{ "+return", KEYMOD_SHIFT | KEY_RETURN },
	{ "+enter", KEYMOD_SHIFT | KEY_RETURN },
	{ "+pause", KEYMOD_SHIFT | KEY_PAUSE },
	{ "+escape", KEYMOD_SHIFT | KEY_ESCAPE },
	{ "+space", KEYMOD_SHIFT | KEY_SPACE },
	{ "+:", KEYMOD_SHIFT | KEY_COLON },
	{ "+,", KEYMOD_SHIFT | KEY_COMMA },
	{ "+-", KEYMOD_SHIFT | KEY_MINUS },
	{ "+.", KEYMOD_SHIFT | KEY_PERIOD },
	{ "+/", KEYMOD_SHIFT | KEY_SLASH },
	{ "+0", KEYMOD_SHIFT | KEY_0 },
	{ "+1", KEYMOD_SHIFT | KEY_1 },
	{ "+2", KEYMOD_SHIFT | KEY_2 },
	{ "+3", KEYMOD_SHIFT | KEY_3 },
	{ "+4", KEYMOD_SHIFT | KEY_4 },
	{ "+5", KEYMOD_SHIFT | KEY_5 },
	{ "+6", KEYMOD_SHIFT | KEY_6 },
	{ "+7", KEYMOD_SHIFT | KEY_7 },
	{ "+8", KEYMOD_SHIFT | KEY_8 },
	{ "+9", KEYMOD_SHIFT | KEY_9 },
	{ "+;", KEYMOD_SHIFT | KEY_SEMICOLON },
	{ "+^", KEYMOD_SHIFT | KEY_HAT },
	{ "+@", KEYMOD_SHIFT | KEY_AT },
	{ "+]", KEYMOD_SHIFT | KEY_RIGHTBRACKET },
	{ "+[", KEYMOD_SHIFT | KEY_LEFTBRACKET },
	{ "+`", KEYMOD_SHIFT | KEY_BACKQUOTE },
	{ "+a", KEYMOD_SHIFT | KEY_A },
	{ "+b", KEYMOD_SHIFT | KEY_B },
	{ "+c", KEYMOD_SHIFT | KEY_C },
	{ "+d", KEYMOD_SHIFT | KEY_D },
	{ "+e", KEYMOD_SHIFT | KEY_E },
	{ "+f", KEYMOD_SHIFT | KEY_F },
	{ "+g", KEYMOD_SHIFT | KEY_G },
	{ "+h", KEYMOD_SHIFT | KEY_H },
	{ "+i", KEYMOD_SHIFT | KEY_I },
	{ "+j", KEYMOD_SHIFT | KEY_J },
	{ "+k", KEYMOD_SHIFT | KEY_K },
	{ "+l", KEYMOD_SHIFT | KEY_L },
	{ "+m", KEYMOD_SHIFT | KEY_M },
	{ "+n", KEYMOD_SHIFT | KEY_N },
	{ "+o", KEYMOD_SHIFT | KEY_O },
	{ "+p", KEYMOD_SHIFT | KEY_P },
	{ "+q", KEYMOD_SHIFT | KEY_Q },
	{ "+r", KEYMOD_SHIFT | KEY_R },
	{ "+s", KEYMOD_SHIFT | KEY_S },
	{ "+t", KEYMOD_SHIFT | KEY_T },
	{ "+u", KEYMOD_SHIFT | KEY_U },
	{ "+v", KEYMOD_SHIFT | KEY_V },
	{ "+w", KEYMOD_SHIFT | KEY_W },
	{ "+x", KEYMOD_SHIFT | KEY_X },
	{ "+y", KEYMOD_SHIFT | KEY_Y },
	{ "+z", KEYMOD_SHIFT | KEY_Z },
	{ "+delete", KEYMOD_SHIFT | KEY_DELETE },
	{ "+n0", KEYMOD_SHIFT | KEY_KP0 },
	{ "+n1", KEYMOD_SHIFT | KEY_KP1 },
	{ "+n2", KEYMOD_SHIFT | KEY_KP2 },
	{ "+n3", KEYMOD_SHIFT | KEY_KP3 },
	{ "+n4", KEYMOD_SHIFT | KEY_KP4 },
	{ "+n5", KEYMOD_SHIFT | KEY_KP5 },
	{ "+n6", KEYMOD_SHIFT | KEY_KP6 },
	{ "+n7", KEYMOD_SHIFT | KEY_KP7 },
	{ "+n8", KEYMOD_SHIFT | KEY_KP8 },
	{ "+n9", KEYMOD_SHIFT | KEY_KP9 },
	{ "+n.", KEYMOD_SHIFT | KEY_KP_PERIOD },
	{ "+n/", KEYMOD_SHIFT | KEY_KP_DIVIDE },
	{ "+n*", KEYMOD_SHIFT | KEY_KP_MULTIPLY },
	{ "+n-", KEYMOD_SHIFT | KEY_KP_MINUS },
	{ "+n+", KEYMOD_SHIFT | KEY_KP_PLUS },
	{ "+nreturn", KEYMOD_SHIFT | KEY_KP_ENTER },
	{ "+nenter", KEYMOD_SHIFT | KEY_KP_ENTER },
	{ "+n=", KEYMOD_SHIFT | KEY_KP_EQUALS },
	{ "+up", KEYMOD_SHIFT | KEY_UP },
	{ "+down", KEYMOD_SHIFT | KEY_DOWN },
	{ "+right", KEYMOD_SHIFT | KEY_RIGHT },
	{ "+left", KEYMOD_SHIFT | KEY_LEFT },
	{ "+insert", KEYMOD_SHIFT | KEY_INSERT },
	{ "+home", KEYMOD_SHIFT | KEY_HOME },
	{ "+end", KEYMOD_SHIFT | KEY_END },
	{ "+pageup", KEYMOD_SHIFT | KEY_PAGEUP },
	{ "+pagedown", KEYMOD_SHIFT | KEY_PAGEDOWN  },
	{ "+f1", KEYMOD_SHIFT | KEY_F1 },
	{ "+f2", KEYMOD_SHIFT | KEY_F2 },
	{ "+f3", KEYMOD_SHIFT | KEY_F3 },
	{ "+f4", KEYMOD_SHIFT | KEY_F4 },
	{ "+f5", KEYMOD_SHIFT | KEY_F5 },
	{ "+f6", KEYMOD_SHIFT | KEY_F6 },
	{ "+f7", KEYMOD_SHIFT | KEY_F7 },
	{ "+f8", KEYMOD_SHIFT | KEY_F8 },
	{ "+f9", KEYMOD_SHIFT | KEY_F9 },
	{ "+f10", KEYMOD_SHIFT | KEY_F10 },
	{ "+f11", KEYMOD_SHIFT | KEY_F11 },
	{ "+f12", KEYMOD_SHIFT | KEY_F12 },
	{ "+f13", KEYMOD_SHIFT | KEY_F13 },
	{ "+f14", KEYMOD_SHIFT | KEY_F14 },
	{ "+f15", KEYMOD_SHIFT | KEY_F15 },
	{ "+numlock", KEYMOD_SHIFT | KEY_NUMLOCK },
	{ "+capslock", KEYMOD_SHIFT | KEY_CAPSLOCK },
	{ "+scrolllock", KEYMOD_SHIFT | KEY_SCROLLOCK },
	{ "+mode", KEYMOD_SHIFT | KEY_MODE },
	{ "+compose", KEYMOD_SHIFT | KEY_COMPOSE },
	{ "+help", KEYMOD_SHIFT | KEY_HELP },
	{ "+print", KEYMOD_SHIFT | KEY_PRINT },
	{ "+sysreq", KEYMOD_SHIFT | KEY_SYSREQ },
	{ "+break", KEYMOD_SHIFT | KEY_BREAK },
	{ "+menu", KEYMOD_SHIFT | KEY_MENU },
	{ "+power", KEYMOD_SHIFT | KEY_POWER },
	{ "+_", KEYMOD_SHIFT | KEY_UNDERSCORE },
	{ "+kana", KEYMOD_SHIFT | KEY_KANA },
	{ "+\\", KEYMOD_SHIFT | KEY_YEN  },
	{ "+xfer", KEYMOD_SHIFT | KEY_XFER },
	{ "+nfer", KEYMOD_SHIFT | KEY_NFER },
	{ "+non-us-backslash", KEYMOD_SHIFT | KEY_NONUSBACKSLASH },
	{ NULL, 0 }
};

/*
	サウンドバッファ長を得る (initの下請け)
*/
static int getSoundBufferLength(int *refresh_rate)
{
	int len;

	for(len = 0x40000000; len != 1 && len > AUDIO_RATE / *refresh_rate; len >>= 1)
		;

	*refresh_rate = AUDIO_RATE / len;
	return len;
}

/*
	エミュレータを初期化する
*/
int init(Z1stat *z1, int argc, char *argv[])
{
	Conf conf[256];
	int ram_size;
	uint16 key;
	const OptTable *p;

	memset(z1, 0, sizeof(*z1));
	z1->cpu.i.user_data = z1;
	z1->cpu.m = z1->memory;
	z1->disk.files_p = z1->disk.files = NULL;

	if(getConfig(conf, sizeof(conf) / sizeof(conf[0]), "z1f9config", argc, argv) == NULL)
		return FALSE;

	/* デバッグモード */
	z1->cpu.i.trace = getOptYesNo(conf, "debug", FALSE);

	/* 機種 */
	switch(getOptTable(conf, "machine", tableMachine, 0)) {
	case MACHINE_FX890P:
		z1->setting.machine = 0x30;
		z1->setting.machine_name = "FX-890P";
		z1->setting.oram_size = z1->setting.iram_size = 0x1000;
		break;
	case MACHINE_FX890P_EN:
		z1->setting.machine = 0x10;
		z1->setting.machine_name = "FX-890P";
		z1->setting.oram_size = z1->setting.iram_size = 0x1000;
		break;
	case MACHINE_Z1:
		z1->setting.machine = 0x20;
		z1->setting.machine_name = "Z-1";
		z1->setting.oram_size = z1->setting.iram_size = 0x0800;
		break;
	default:
		z1->setting.machine = 0x20;
		z1->setting.machine_name = "Z-1GR";
		z1->setting.oram_size = z1->setting.iram_size = 0x0800;
		break;
	}

	/* ROMイメージファイル */
	setHomeDir(z1->setting.path_rom, getOptText(conf, "rom_path", ""));

	/* RAMイメージファイル */
	setHomeDir(z1->setting.path_ram, getOptText(conf, "ram_path", "./ram.bin"));

	/* LCD倍率 */
	z1->setting.zoom = getOptInt(conf, "zoom", 3);

	/* LCD階調数 */
	if((z1->setting.scales = getOptInt(conf, "lcd_scales", 2)) == 1)
		z1->setting.scales = 2;
	else if(z1->setting.scales < 0)
		z1->setting.scales = 0;

	/* シリアルポート 出力先 */
	setIOData(&z1->rs_send, getOptText(conf, "sio_out", "./sio_out.txt"));

	/* シリアルポート 入力元 */
	setIOData(&z1->rs_receive, getOptText(conf, "sio_in", "./sio_in.txt"));

	/* プリンタポート 出力先 */
	setIOData(&z1->printer, getOptText(conf, "printer", "./print.txt"));

	/* 仮想フロッピーディスク ディレクトリ */
	strcpy(z1->disk.dir, getOptText(conf, "floppy_dir", "./"));

	/* キー配置 */
	for(p = tableZkey; p->string != NULL; p++) {
		if(p->value & ~ZKEYMOD_MASK)
			continue;
		if((key = getOptTable(conf, p->string, tableKey, 0)) == 0)
			continue;
		keyConv[key & ~KEYMOD_MASK] = p->value;
		keyConvAlt[key & ~KEYMOD_MASK] = p->value;
		keyConvCtrl[key & ~KEYMOD_MASK] = p->value;
		keyConvShift[key & ~KEYMOD_MASK] = p->value;
	}

	for(p = tableZkey; p->string != NULL; p++) {
		if((key = getOptTable(conf, p->string, tableKey, 0)) == 0)
			continue;
		switch(key & KEYMOD_MASK) {
		case 0:
			keyConv[key & ~KEYMOD_MASK] = p->value;
			break;
		case KEYMOD_ALT:
			if(p->value & ZKEYMOD_SHIFT)
				keyConvAlt[key & ~KEYMOD_MASK] = p->value;
			else
				keyConvAlt[key & ~KEYMOD_MASK] = (p->value & ~ZKEYMOD_MASK) | ZKEYMOD_NOSHIFT;
			break;
		case KEYMOD_CTRL:
			if(p->value & ZKEYMOD_SHIFT)
				keyConvCtrl[key & ~KEYMOD_MASK] = p->value;
			else
				keyConvCtrl[key & ~KEYMOD_MASK] = (p->value & ~ZKEYMOD_MASK) | ZKEYMOD_NOSHIFT;
			break;
		case KEYMOD_SHIFT:
			if(p->value & ZKEYMOD_SHIFT)
				keyConvShift[key & ~KEYMOD_MASK] = p->value & ~ZKEYMOD_MASK;
			else
				keyConvShift[key & ~KEYMOD_MASK] = (p->value & ~ZKEYMOD_MASK) | ZKEYMOD_NOSHIFT;
			break;
		}
	}

	/* CPUクロック周波数 */
	z1->setting.cpu_clock = getOptInt(conf, "clock", 3686400);

	/* 内蔵/拡張RAM容量 */
	ram_size = getOptInt(conf, "ram_size", 0) * 0x40;
	if (ram_size < z1->setting.iram_size)
		ram_size = z1->setting.iram_size;
	else if (ram_size > 0x4000)
		ram_size = 0x4000;
	if (ram_size - z1->setting.iram_size == 0x200)
		z1->setting.oram_size = ram_size;
	else if (ram_size - z1->setting.iram_size == 0x800)
		z1->setting.oram_size = ram_size;
	else
		z1->setting.iram_size = z1->setting.oram_size = ram_size;

	/* I/O 更新周期 */
	z1->setting.refresh_rate = getOptInt(conf, "refresh", 30);

	/* ジョイスティック */
	z1->joy.right = getOptTable(conf, "joy_right", tableZkey, 0);
	z1->joy.left = getOptTable(conf, "joy_left", tableZkey, 0);
	z1->joy.up = getOptTable(conf, "joy_up", tableZkey, 0);
	z1->joy.down = getOptTable(conf, "joy_down", tableZkey, 0);
	z1->joy.button[0] = getOptTable(conf, "joy_button1", tableZkey, 0);
	z1->joy.button[1] = getOptTable(conf, "joy_button2", tableZkey, 0);
	z1->joy.button[2] = getOptTable(conf, "joy_button3", tableZkey, 0);
	z1->joy.button[3] = getOptTable(conf, "joy_button4", tableZkey, 0);
	z1->joy.button[4] = getOptTable(conf, "joy_button5", tableZkey, 0);
	z1->joy.button[5] = getOptTable(conf, "joy_button6", tableZkey, 0);
	z1->joy.button[6] = getOptTable(conf, "joy_button7", tableZkey, 0);
	z1->joy.button[7] = getOptTable(conf, "joy_button8", tableZkey, 0);
	z1->joy.button[8] = getOptTable(conf, "joy_button9", tableZkey, 0);
	z1->joy.button[9] = getOptTable(conf, "joy_button10", tableZkey, 0);
	z1->joy.button[10] = getOptTable(conf, "joy_button11", tableZkey, 0);
	z1->joy.button[11] = getOptTable(conf, "joy_button12", tableZkey, 0);
	z1->joy.button[12] = getOptTable(conf, "joy_button13", tableZkey, 0);
	z1->joy.button[13] = getOptTable(conf, "joy_button14", tableZkey, 0);
	z1->joy.button[14] = getOptTable(conf, "joy_button15", tableZkey, 0);
	z1->joy.button[15] = getOptTable(conf, "joy_button16", tableZkey, 0);

	/* ブザー */
	if(getOptYesNo(conf, "buzzer", TRUE)) {
		z1->sound.len = getSoundBufferLength(&z1->setting.refresh_rate);
		z1->sound.buffer[0] = calloc(z1->sound.len, 1);
		z1->sound.buffer[1] = calloc(z1->sound.len, 1);
	}
	return initDepend(z1);
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
