/*
	CASIO Z-1/FX-890P emulator
	I/O
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "z1.h"

/*
	Inportをエミュレートする (8bit)
*/
uint8 i86inp8(I86stat *i86, uint16 port)
{
	Z1stat *z1 = i86->i.user_data;
	uint32 size;
	uint16 v;
	uint8 data, f, a[16];

	/*
	printf("IO IN %04x\n", port);
	*/
	i86->i.op_states += 12;

	switch(port) {
	case 0x0008: /* 割込マスク */
		return
		(z1->timer.control & 0x08 ? 0x01: 0) |
		(z1->sio.control & 0x08 ? 0x04: 0) |
		(z1->card.control & 0x08 ? 0x08: 0) |
		(z1->key.control & 0x08 ? 0x10: 0) |
		(z1->sw.control & 0x08 ? 0x20: 0) |
		0x40 |
		0x80;
	case 0x0009:
		return 0x00;

	case 0x000c: /* 割込中 */
		return
		(z1->timer.intr ? 0x01: 0) |
		(z1->sio.intr ? 0x04: 0) |
		(z1->card.intr ? 0x08: 0) |
		(z1->key.intr ? 0x10: 0) |
		(z1->sw.intr ? 0x20: 0);
	case 0x000d:
		return 0x00;

	case 0x0012: /* タイマ 割込コントローラ */
		return *LOW(z1->timer.control);
	case 0x0013:
		return *HIGH(z1->timer.control);

	case 0x0014: /* シリアルポート 割込コントローラ */
		return *LOW(z1->sio.control);
	case 0x0015:
		return *HIGH(z1->sio.control);

	case 0x0016: /* 外部割込4(カードエッジ) 割込コントローラ */
		return *LOW(z1->card.control);
	case 0x0017:
		return *HIGH(z1->card.control);

	case 0x0018: /* 外部割込0(キー) 割込コントローラ */
		return *LOW(z1->key.control);
	case 0x0019:
		return *HIGH(z1->key.control);

	case 0x001a: /* 外部割込1(電源スイッチ) 割込コントローラ */
		return *LOW(z1->sw.control);
	case 0x001b:
		return *HIGH(z1->sw.control);

	case 0x001c: /* 外部割込2 割込コントローラ */
		return 0x0f;
	case 0x001d:
		return 0x00;

	case 0x001e: /* 外部割込3 割込コントローラ */
		return 0x0f;
	case 0x001f:
		return 0x00;

	case 0x0030: /* タイマ0 カウンタ */
		return getTimerCount(z1, &z1->timer.t0) & 0xff;
	case 0x0031:
		return getTimerCount(z1, &z1->timer.t0) >> 8;

	case 0x0032: /* タイマ0 間隔A */
		return *LOW(z1->timer.t0.interval_a);
	case 0x0033:
		return *HIGH(z1->timer.t0.interval_a);

	case 0x0034: /* タイマ0 間隔B */
		return *LOW(z1->timer.t0.interval_b);
	case 0x0035:
		return *HIGH(z1->timer.t0.interval_b);

	case 0x0036: /* タイマ0 コントローラ */
		return *LOW(z1->timer.t0.control);
	case 0x0037:
		return *HIGH(z1->timer.t0.control);

	case 0x0038: /* タイマ1 カウンタ */
		return getTimerCount(z1, &z1->timer.t1) & 0xff;
	case 0x0039:
		return getTimerCount(z1, &z1->timer.t1) >> 8;

	case 0x003a: /* タイマ1 間隔A */
		return *LOW(z1->timer.t1.interval_a);
	case 0x003b:
		return *HIGH(z1->timer.t1.interval_a);

	case 0x003c: /* タイマ1 間隔B */
		return *LOW(z1->timer.t1.interval_b);
	case 0x003d:
		return *HIGH(z1->timer.t1.interval_b);

	case 0x003e: /* タイマ1 コントローラ */
		return *LOW(z1->timer.t1.control);
	case 0x003f:
		return *HIGH(z1->timer.t1.control);

	case 0x0040: /* タイマ2 カウンタ */
		return getTimerCount(z1, &z1->timer.t2) & 0xff;
	case 0x0041:
		return getTimerCount(z1, &z1->timer.t2) >> 8;

	case 0x0042: /* タイマ2 間隔A */
		return *LOW(z1->timer.t2.interval_a);
	case 0x0043:
		return *HIGH(z1->timer.t2.interval_a);

	case 0x0046: /* タイマ2 コントローラ */
		return *LOW(z1->timer.t2.control);
	case 0x0047:
		return *HIGH(z1->timer.t2.control);

	case 0x005a: /* バッテリー容量 */
		return 0x04 | z1->setting.machine;
	case 0x005b:
		return 0x00;

	case 0x0060: /* シリアルポート ボーレート */
		return *LOW(z1->sio.baud);
	case 0x0061:
		return *HIGH(z1->sio.baud);

	case 0x0064: /* シリアルポート 設定 */
		return *LOW(z1->sio.settings);
	case 0x0065:
		return *HIGH(z1->sio.settings);

	case 0x0066: /* シリアルポート ステータス */
		return 0x08;
	case 0x0067:
		return 0x00;

	case 0x0068: /* シリアルポート 受信 */
		receiveIOData(&z1->rs_receive, &data);
		return data;
	case 0x0069:
		return 0x00;

	case 0x0082: /* 内蔵RAM容量 */
		return *LOW(z1->setting.iram_size);
	case 0x0083:
		return *HIGH(z1->setting.iram_size);

	case 0x0086: /* 拡張RAM容量 */
		return *LOW(z1->setting.oram_size);
	case 0x0087:
		return *HIGH(z1->setting.oram_size);

	case 0x00a2: /* ??? */
		return 0x0a;
	case 0x00a3:
		return 0x80;

	case 0x00a6: /* ??? */
		return 0xce;
	case 0x00a7:
		return 0xff;

	case 0x00b8: /* 電源コントローラ */
		return *LOW(z1->power);
	case 0x00b9:
		return *HIGH(z1->power);

	case 0x0202: /* キー状態 */
	case 0x0203:
		/*i86->i.op_states += 24;*/
		v =
		(z1->key.strobe & 0x0001 ? z1->key.matrix[0]: 0) |
		(z1->key.strobe & 0x0002 ? z1->key.matrix[1]: 0) |
		(z1->key.strobe & 0x0004 ? z1->key.matrix[2]: 0) |
		(z1->key.strobe & 0x0008 ? z1->key.matrix[3]: 0) |
		(z1->key.strobe & 0x0010 ? z1->key.matrix[4]: 0) |
		(z1->key.strobe & 0x0020 ? z1->key.matrix[5]: 0) |
		(z1->key.strobe & 0x0040 ? z1->key.matrix[6]: 0) |
		(z1->key.strobe & 0x0080 ? z1->key.matrix[7]: 0) |
		(z1->key.strobe & 0x0100 ? z1->key.matrix[8]: 0) |
		(z1->key.strobe & 0x0200 ? z1->key.matrix[9]: 0) |
		(z1->key.strobe & 0x0400 ? z1->key.matrix[10]: 0) |
		z1->key.matrix[11];
		if(!v)
			v = 0x8000;
		return (port == 0x0202 ? v & 0xff: v >> 8);

	case 0x0204: /* キー割込コントロール */
		return 0x04;
	case 0x0205:
		return 0x00;

	case 0x0220: /* 演算 */
		opXY(z1->cal.op, &i86->m[0x0400], &i86->m[0x0410], a, &f);
		return f;
	case 0x0221:
		return 0x00;

	case 0x0240: /* シリアル・パラレルポート ステータス */
		return 0x00;
	case 0x0241:
		/* DTR RTS INIT STB CTR DSR CD BUSY */
		return 0xfe;

	case 0x0274: /* ??? */
		return 0x76;
	case 0x0275:
		return 0x75;

	case 0x02a6: /* FDD */
		/* 結果 */
		/*printf("FDD DATA? CMD=%02x, COUNT=%d, DATA=%02x, ERR=%02x\n", z1->fdd.cmd, z1->disk.result_count, z1->disk.data, z1->fdd.err);*/
		switch(z1->fdd.cmd) {
		case 0x03: /* ファイル検索結果 */
			z1->fdd.err = getFoundFdFile(&z1->disk, &z1->disk.data);
			return z1->disk.data;
		default:
			return z1->disk.data;
		}
	case 0x02a7:
		/* 実行 01:エラー, 02:コマンド書込正常, 04:コマンド正常開始, 08:コマンド正常完了, 10:デバイス正常 */
		if(z1->disk.result_count < 0)
			return 0x1e;
		/*printf("FDD STAT? CMD=%02x, PARAM=%s, LEN=%d, COUNT=%d, ERR=%02x\n", z1->fdd.cmd, z1->fdd.param, (int )(z1->fdd.param_p - z1->fdd.param), z1->disk.result_count, z1->fdd.err);*/

		switch(z1->fdd.cmd) {
		case 0x10: /* ディスク空き容量 */
			z1->fdd.err = getFdFreeSize(&z1->disk, &size);

			if(z1->disk.result_count == 0)
				z1->disk.data = z1->fdd.err;
			else if(z1->disk.result_count == 1)
				z1->disk.data = size & 0xff;
			else if(z1->disk.result_count == 2)
				z1->disk.data = (size >> 8) & 0xff;
			else if(z1->disk.result_count == 3)
				z1->disk.data = (size >> 16) & 0xff;
			else if(z1->disk.result_count == 4)
				z1->disk.data = (size >> 24) & 0xff;
			else
				z1->disk.data = 0x00;
			break;
		case 0x12: /* ファイル検索 */
			if(z1->disk.result_count == 0)
				z1->disk.data = z1->fdd.err = 0x00;
			else if(z1->disk.result_count == 1)
				z1->fdd.err = findFdFile(&z1->disk, &z1->fdd.param[0], &z1->disk.data);
			else if(z1->disk.result_count == 2)
				z1->disk.data = 0x00;
			else if(z1->disk.result_count == 3)
				z1->disk.data = 0x00;
			break;
		case 0x13: /* ファイル一括読込オープン */
			if(z1->disk.result_count == 0)
				z1->disk.data = z1->fdd.err;
			else if(z1->disk.result_count == 1)
				z1->disk.data = z1->disk.size & 0xff;
			else if(z1->disk.result_count == 2)
				z1->disk.data = (z1->disk.size >> 8) & 0xff;
			else if(z1->disk.result_count == 3)
				z1->disk.data = (z1->disk.size >> 16) & 0xff;
			else if(z1->disk.result_count == 4)
				z1->disk.data = (z1->disk.size >> 24) & 0xff;
			else if(z1->disk.result_count < 4 + z1->disk.size)
				z1->fdd.err = readFdFile(&z1->disk, &z1->disk.data);
			else
				z1->disk.data = 0x00;
			break;
		case 0x1b: /* ファイル読込/書込 */
			if(z1->disk.result_count == 0)
				z1->disk.data = z1->fdd.err;
			else if(z1->disk.result_count == 1)
				*LOW(z1->disk.len) = z1->disk.data = z1->disk.size - z1->disk.pos >= 0x100 ? 0x00: z1->disk.size - z1->disk.pos;
			else if(z1->disk.result_count == 2)
				*HIGH(z1->disk.len) = z1->disk.data = z1->disk.size - z1->disk.pos >= 0x100 ? 0x01: 0x00;
			else if(z1->disk.result_count < 3 + MIN(z1->disk.len, 256))
				z1->fdd.err = readFdFile(&z1->disk, &z1->disk.data);
			else
				z1->disk.data = 0x00;
			break;
		default:
			if(z1->disk.result_count == 0)
				z1->disk.data = z1->fdd.err;
			else
				z1->disk.data = 0x00;
			break;
		}
		z1->disk.result_count++;
		return (z1->fdd.err ? 0x01: 0x1e);

	default:
		/*printf("IO IN %04x\n", port);*/
		return 0;
	}
}

/*
	Inportをエミュレートする (16bit)
*/
uint16 i86inp16(I86stat *i86, uint16 port)
{
	return ((uint16 )i86inp8(i86, port + 1) << 8) | i86inp8(i86, port);
}

/*
	Outportをエミュレートする (8bit)
*/
int i86out8(I86stat *i86, uint16 port, uint8 x)
{
	Z1stat *z1 = i86->i.user_data;
	uint8 f, a[16];

	/*printf("OUT8  %04x,%02x\n", port, x);*/

	i86->i.op_states += 12;

	switch(port) {
	case 0x0002: /* 割込終了 */
		return FALSE;
	case 0x0003:
		if(x & 0x80) {
			if(z1->timer.intr > 0)
				z1->timer.intr--;
			if(z1->sio.intr > 0)
				z1->sio.intr--;
			if(z1->card.intr > 0)
				z1->card.intr--;
			if(z1->key.intr > 0)
				z1->key.intr--;
			if(z1->sw.intr > 0)
				z1->sw.intr--;
		}
		return TRUE;

	case 0x000c: /* 割込中 */
		return FALSE;
	case 0x000d:
		return FALSE;

	case 0x0008: /* 割込マスク */
		if(x & 0x01)
			*LOW(z1->timer.control) |= 0x08;
		else
			*LOW(z1->timer.control) &= ~0x08;
		if(x & 0x04)
			*LOW(z1->sio.control) |= 0x08;
		else
			*LOW(z1->sio.control) &= ~0x08;
		if(x & 0x08)
			*LOW(z1->card.control) |= 0x08;
		else
			*LOW(z1->card.control) |= 0x08;
		if(x & 0x10)
			*LOW(z1->key.control) |= 0x08;
		else
			*LOW(z1->key.control) &= ~0x08;
		if(x & 0x20)
			*LOW(z1->sw.control) |= 0x08;
		else
			*LOW(z1->sw.control) &= ~0x08;
		return TRUE;
	case 0x0009:
		return FALSE;

	case 0x0012: /* タイマ 割込コントローラ */
		*LOW(z1->timer.control) = x & 0x0f;
		return TRUE;
	case 0x0013:
		*HIGH(z1->timer.control) = 0;
		return FALSE;

	case 0x0014: /* シリアルポート 割込コントローラ */
		*LOW(z1->sio.control) = x & 0x0f;
		return FALSE;
	case 0x0015:
		*HIGH(z1->sio.control) = 0;
		return FALSE;

	case 0x0016: /* 外部割込4(カードエッジ) 割込コントローラ */
		*LOW(z1->card.control) = x & 0x7f;
		return FALSE;
	case 0x0017:
		*HIGH(z1->card.control) = 0;
		return FALSE;

	case 0x0018: /* 外部割込0(キー) 割込コントローラ */
		*LOW(z1->key.control) = x & 0x7f;
		return FALSE;
	case 0x0019:
		*HIGH(z1->key.control) = 0;
		return FALSE;

	case 0x001a: /* 外部割込1(電源スイッチ) 割込コントローラ */
		*LOW(z1->sw.control) = x & 0x7f;
		return FALSE;
	case 0x001b:
		*HIGH(z1->sw.control) = 0;
		return FALSE;

	case 0x001c: /* 外部割込2 割込コントローラ */
		return FALSE;
	case 0x001d:
		return FALSE;

	case 0x001e: /* 外部割込3 割込コントローラ */
		return FALSE;
	case 0x001f:
		return FALSE;

	case 0x0030: /* タイマ0 カウンタ */
		setTimerCount(z1, &z1->timer.t0, (getTimerCount(z1, &z1->timer.t0) & 0xff00) | x);
		return TRUE;
	case 0x0031:
		setTimerCount(z1, &z1->timer.t0, (getTimerCount(z1, &z1->timer.t0) & 0x00ff) | ((int )x << 8));
		return TRUE;

	case 0x0032: /* タイマ0 間隔A */
		*LOW(z1->timer.t0.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t0, 0);*/
		return TRUE;
	case 0x0033:
		*HIGH(z1->timer.t0.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t0, 0);*/
		return TRUE;

	case 0x0034: /* タイマ0 間隔B */
		*LOW(z1->timer.t0.interval_b) = x;
		/*setTimerCount(z1, &z1->timer.t0, 0);*/
		return TRUE;
	case 0x0035:
		*HIGH(z1->timer.t0.interval_b) = x;
		/*setTimerCount(z1, &z1->timer.t0, 0);*/
		return TRUE;

	case 0x0036: /* タイマ0 コントローラ */
		*LOW(z1->timer.t0.control) = x & 0x3f;
		return TRUE;
	case 0x0037:
		*HIGH(z1->timer.t0.control) = x & 0xb0;
		if(z1->timer.t0.control & 0x8000)
			setTimerCount(z1, &z1->timer.t0, 0);
		return TRUE;

	case 0x0038: /* タイマ1 カウンタ */
		setTimerCount(z1, &z1->timer.t1, (getTimerCount(z1, &z1->timer.t1) & 0xff00) | x);
		return TRUE;
	case 0x0039:
		setTimerCount(z1, &z1->timer.t1, (getTimerCount(z1, &z1->timer.t1) & 0x00ff) | ((int )x << 8));
		return TRUE;

	case 0x003a: /* タイマ1 間隔A */
		*LOW(z1->timer.t1.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t1, 0);*/
		return TRUE;
	case 0x003b:
		*HIGH(z1->timer.t1.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t1, 0);*/
		return TRUE;

	case 0x003c: /* タイマ1 間隔B */
		*LOW(z1->timer.t1.interval_b) = x;
		/*setTimerCount(z1, &z1->timer.t1, 0);*/
		return TRUE;
	case 0x003d:
		*HIGH(z1->timer.t1.interval_b) = x;
		/*setTimerCount(z1, &z1->timer.t1, 0);*/
		return TRUE;

	case 0x003e: /* タイマ1 コントローラ */
		*LOW(z1->timer.t1.control) = x & 0x3f;
		return TRUE;
	case 0x003f:
		*HIGH(z1->timer.t1.control) = x & 0xb0;
		if(z1->timer.t1.control & 0x8000)
			setTimerCount(z1, &z1->timer.t1, 0);
		return TRUE;

	case 0x0040: /* タイマ2 カウンタ */
		setTimerCount(z1, &z1->timer.t2, (getTimerCount(z1, &z1->timer.t2) & 0xff00) | x);
		return TRUE;
	case 0x0041:
		setTimerCount(z1, &z1->timer.t2, (getTimerCount(z1, &z1->timer.t2) & 0x00ff) | ((int )x << 8));
		return TRUE;

	case 0x0042: /* タイマ2 間隔A */
		*LOW(z1->timer.t2.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t2, 0);*/
		return TRUE;
	case 0x0043:
		*HIGH(z1->timer.t2.interval_a) = x;
		/*setTimerCount(z1, &z1->timer.t2, 0);*/
		return TRUE;

	case 0x0046: /* タイマ2 コントローラ */
		*LOW(z1->timer.t2.control) = x & 0x21;
		return TRUE;
	case 0x0047:
		*HIGH(z1->timer.t2.control) = x & 0xa0;
		if(z1->timer.t2.control & 0x8000)
			setTimerCount(z1, &z1->timer.t2, 0);
		return TRUE;

	case 0x0060: /* シリアルポート ボーレート */
		*LOW(z1->sio.baud) = x;
		/*printf("clock=%d, baud=%04x\n", z1->sio.baud & 0x8000 ? 1: 0, z1->sio.baud & 0x7fff);*/
		return FALSE;
	case 0x0061:
		*HIGH(z1->sio.baud) = x;
		/*printf("clock=%d, baud=%04x\n", z1->sio.baud & 0x8000 ? 1: 0, z1->sio.baud & 0x7fff);*/
		return FALSE;

	case 0x0064: /* シリアルポート 設定 */
		*LOW(z1->sio.settings) = x;
		/*
		printf("COM %s %s %s %s %s %s M%d\n",
		z1->sio.settings & 0x0100 ? "SBRK": "-",
		z1->sio.settings & 0x0080 ? "TB8": "-",
		z1->sio.settings & 0x0040 ? "CEN": "-",
		z1->sio.settings & 0x0020 ? "REN": "-",
		z1->sio.settings & 0x0010 ? "E": "O",
		z1->sio.settings & 0x0008 ? "N": "P",
		z1->sio.settings & 0x0007);
		*/
		return FALSE;
	case 0x0065:
		*HIGH(z1->sio.settings) = x;
		/*
		printf("COM %s %s %s %s %s %s M%d\n",
		z1->sio.settings & 0x0100 ? "SBRK": "-",
		z1->sio.settings & 0x0080 ? "TB8": "-",
		z1->sio.settings & 0x0040 ? "CEN": "-",
		z1->sio.settings & 0x0020 ? "REN": "-",
		z1->sio.settings & 0x0010 ? "E": "O",
		z1->sio.settings & 0x0008 ? "N": "P",
		z1->sio.settings & 0x0007);
		*/
		return FALSE;

	case 0x006a: /* シリアルポート 送信 */
		/*printf("COM SEND %c(%02x)\n", x, x);*/
		sendIOData(&z1->rs_send, x);
		z1->sio.sent = TRUE;
		return TRUE;
	case 0x006b:
		return FALSE;

	case 0x00b8: /* 電源コントローラ */
		*LOW(z1->power) = x & 0x03;
		return x & 0x01;
	case 0x00b9:
		*HIGH(z1->power) = 0;
		return FALSE;

	case 0x0200: /* キーストローブ */
		/*i86->i.op_states += 12;*/
		*LOW(z1->key.strobe) = x;
		return FALSE;
	case 0x0201:
		/*i86->i.op_states += 12;*/
		*HIGH(z1->key.strobe) = x;
		return FALSE;

	case 0x0204: /* キー割込コントロール */
		if((z1->key.key_control & 0x01) && !(x & 0x01))
			z1->key.key_intr = FALSE;
		*LOW(z1->key.key_control) = x;
		return FALSE;
	case 0x0205:
		*HIGH(z1->key.key_control) = 0;
		return FALSE;

	case 0x0206: /* ブザー */
		/*printf("BUZZER=%02x\n", x);*/
		z1->buzzer = x;
		writeSound(z1, z1->buzzer);
		return FALSE;
	case 0x0207:
		return FALSE;

	case 0x0220: /* 演算 */
		/*z1->cpu.i.op_states += z1->setting.cpu_clock / 20340;*/ /* ??? */

		z1->cal.op = x;
		opXY(z1->cal.op, &i86->m[0x400], &i86->m[0x410], a, &f);

		/*
		printf("OP=%02x\n", z1->cal.op);
		printf("X=%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		i86->m[0x0400],
		i86->m[0x0401],
		i86->m[0x0402],
		i86->m[0x0403],
		i86->m[0x0404],
		i86->m[0x0405],
		i86->m[0x0406],
		i86->m[0x0407],
		i86->m[0x0408]);
		printf("Y=%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		i86->m[0x0410],
		i86->m[0x0411],
		i86->m[0x0412],
		i86->m[0x0413],
		i86->m[0x0414],
		i86->m[0x0415],
		i86->m[0x0416],
		i86->m[0x0417],
		i86->m[0x0418]);
		*/

		if(z1->cal.op & 0x08) { /* 仮数部をシフトする */
			switch(z1->cal.op & 0x02) {
			case 0x00: /* X */
				memcpy(z1->cal.reg, &i86->m[0x400], 7);
				break;
			case 0x02: /* Y */
				memcpy(z1->cal.reg, &i86->m[0x410], 7);
				break;
			}
			memcpy(&z1->cal.reg[7], &a[7], 2);

			switch(z1->cal.op & 0x04) {
			case 0x00: /* 仮数部を左シフト(10倍)する */
				encodeMan(z1->cal.reg, decodeMan(z1->cal.reg) * 10LL);
				break;
			case 0x04: /* 仮数部を右シフト(10分の1)する */
				encodeMan(z1->cal.reg, decodeMan(z1->cal.reg) / 10LL);
				break;
			}
		} else if(z1->cal.op & 0x80) /* 演算結果をレジスタに代入する */
			memcpy(z1->cal.reg, a, 9);
		else { /* レジスタに代入しない */
			memset(&z1->cal.reg[0], 0x03, 1);
			memset(&z1->cal.reg[1], 0x00, 8);
		}
		return FALSE;
	case 0x0221:
		switch(x & 0x80) {
		case 0x80: /* XYを交換する */
			memcpy(a, &i86->m[0x0400], 9);
			memcpy(&i86->m[0x0400], &i86->m[0x0410], 9);
			memcpy(&i86->m[0x0410], a, 9);
			break;
		}

		switch(x & 0x0f) {
		case 0x04: /* Xにレジスタ値を代入する */
			memcpy(&i86->m[0x0400], z1->cal.reg, 9);
			break;
		case 0x05: /* Yにレジスタ値を代入する */
			memcpy(&i86->m[0x0410], z1->cal.reg, 9);
			break;
		case 0x08: /* 演算結果モードに移行する */
			z1->cal.map = 1;
			break;
		case 0x0e: /* 演算結果モードを解除する */
			z1->cal.map = 0;
			break;
		}

		switch(x & 0x11) {
		case 0x10: /* Xを初期化する */
			memset(&i86->m[0x0400], 0x00, 9);
			break;
		case 0x11: /* Yを初期化する */
			memset(&i86->m[0x0410], 0x00, 9);
			break;
		}
		return FALSE;

	 case 0x0240: /* シリアル・パラレルポート ステータス */
	 	/* 1 0 0 0 DTR RTS INIT STB */
	 	/*printf("COM/PRN %s %s %s %s %s %s %s %s\n",
	 	x & 0x80 ? "1": "0",
	 	x & 0x40 ? "1": "0",
	 	x & 0x20 ? "1": "0",
	 	x & 0x10 ? "1": "0",
	 	x & 0x08 ? "DTR": "-",
	 	x & 0x04 ? "RTS": "-",
	 	x & 0x02 ? "INIT": "-",
	 	x & 0x01 ? "STB": "-");*/
	 	return FALSE;
	 case 0x0241:
	 	/*printf("COM/PRN %02x\n", x);*/
	 	return FALSE;

	case 0x0280: /* パラレルポート 出力 */
		/*printf("PRN SEND %c(%02x)\n", x, x);*/
		if(x != 0xff)
			sendIOData(&z1->printer, x);
		return FALSE;
	case 0x0281:
		return FALSE;

	case 0x02a6: /* FDD */
		/* パラメータ */
		if(z1->fdd.param_p == NULL)
			z1->fdd.param_p = z1->fdd.param;
		else if(z1->fdd.param_p < &z1->fdd.param[sizeof(z1->fdd.param) - 1])
			z1->fdd.param_p++;
		*z1->fdd.param_p = x;
		/*printf("FDD POS=%d, PARAM=%02x(%c)\n", (int)(z1->fdd.param_p - z1->fdd.param), x, x);*/

		switch(z1->fdd.cmd) {
		case 0x12: /* ファイル検索 */
			if(z1->fdd.param_p == &z1->fdd.param[10])
				z1->disk.result_count = 0;
			break;
		case 0x13: /* ファイル一括読込オープン */
			if(z1->fdd.param_p == &z1->fdd.param[10]) {
				z1->fdd.err = openFdFile(&z1->disk, &z1->fdd.param[0], (uint8 *)"R");
				z1->disk.result_count = 0;
			}
			if(*z1->fdd.param_p == 0x00) {
				z1->fdd.err = closeFdFile(&z1->disk);
				z1->fdd.err = openFdFile(&z1->disk, &z1->fdd.param[0], (uint8 *)"R");
				z1->disk.result_count = 4;
			}
			else if(*z1->fdd.param_p == 0xff) {
				z1->fdd.err = closeFdFile(&z1->disk);
				z1->disk.result_count = -1;
			}
			break;
		case 0x14: /* ファイル一括書込オープン */
			if(z1->fdd.param_p == &z1->fdd.param[10]) {
				z1->fdd.err = openFdFile(&z1->disk, &z1->fdd.param[0], (uint8 *)"W");
				z1->disk.result_count = 0;
			}
			if(z1->fdd.param_p >= &z1->fdd.param[19]) {
				z1->fdd.err = writeFdFile(&z1->disk, x, &z1->fdd.param[15]);
				z1->disk.result_count = 0;
			}
			break;
		case 0x15: /* ファイル名変更 */
			if(z1->fdd.param_p == &z1->fdd.param[21]) {
				z1->fdd.err = renameFdFile(&z1->disk, &z1->fdd.param[0], &z1->fdd.param[11]);
				z1->disk.result_count = 0;
			}
			break;
		case 0x16: /* ファイルの削除 */
			if(z1->fdd.param_p == &z1->fdd.param[10]) {
				z1->fdd.err = deleteFdFile(&z1->disk, &z1->fdd.param[0]);
				z1->disk.result_count = 0;
			}
			break;
		case 0x19: /* ファイルオープン */
			if(z1->fdd.param_p == &z1->fdd.param[16]) {
				z1->fdd.err = openFdFile(&z1->disk, &z1->fdd.param[0], &z1->fdd.param[12]);
				z1->disk.result_count = 0;
			}
			break;
		case 0x1a: /* ファイルクローズ */
			if(z1->fdd.param_p == &z1->fdd.param[0]) {
				z1->fdd.err = closeFdFile(&z1->disk);
				z1->disk.result_count = 0;
			}
			break;
		case 0x1b: /* ファイル読込/書込 */
			if(z1->fdd.param_p == &z1->fdd.param[1] && z1->fdd.param[1] == 'R')
				z1->disk.result_count = 0;
			else if(z1->fdd.param_p >= &z1->fdd.param[4] && z1->fdd.param[1] == 'W') {
				z1->fdd.err = writeFdFile(&z1->disk, x, NULL);
				z1->disk.result_count = 0;
			}
			break;
		}
		return FALSE;
	case 0x02a7:
		/* コマンド */
		z1->fdd.cmd = x;
		memset(z1->fdd.param, 0, sizeof(z1->fdd.param));
		z1->fdd.param_p = NULL;
		z1->disk.result_count = -1;
		z1->disk.data = 0x00;
		/*printf("FDD COMD=%02x(%c)\n", z1->fdd.cmd, z1->fdd.cmd);*/

		switch(z1->fdd.cmd) {
		case 0x01: /* チェック */
			z1->disk.data = 0x00;
			break;
		case 0x10: /* ディスク空き容量 */
			z1->disk.result_count = 0;
			break;
		case 0x11: /* ディスクのフォーマット */
			z1->fdd.err = formatFd(&z1->disk);
			break;
		}
		return FALSE;

 	default:
 		/*
		printf("IO OUT %04x,%02x\n", port, x);
		*/
		return FALSE;
	}
}

/*
	Outportをエミュレートする (16bit)
*/
int i86out16(I86stat *i86, uint16 port, uint16 x)
{
	int result;

	/*printf("OUT16 %04x,%04x\n", port, x);*/

	result = i86out8(i86, port, x & 0xff);
	result |= i86out8(i86, port + 1, x >> 8);
	return result;
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
