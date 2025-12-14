CASIO Z-1/FX-890Pエミュレータ z1f9 マニュアル


* 概要 *
 z1f9はCASIO Z-1/FX-890Pのエミュレータです.
 使用するためには実機のROMイメージが必要です.

 ROMイメージがない場合はBIOSコール(INT 41H)をエミュレートしますが, ほとんどの
 機能が実装されていません. 実装されていない場合はそのBIOSコールを無視します.


* インストール *
 Linux, MacOSXでソースからmakeする場合
 1. z1g9.tgzを展開する.
 2. makeを実行する.
    コンパイルには SDL2.0 の Development library が必要である.
    http://www.libsdl.org/からダウンロードできる.
    (SDL1.2でもよいがMakefileを修正する必要がある. また, その場合は一部の機能
    が使えない.)
 3. rootユーザでmake installを実行する. (/usr/local/binにコピーされる.)
 4. 設定ファイルz1f9configを.z1f9configに名前を変えてホームディレクトリにコピ
    ーする.
 5. ROMイメージファイル(rom.txt)をz1configで設定したディレクトリ(デフォルトは
    ~/z1f9/)にコピーする.

 Windowsの場合
 1. z1f9win32.zipを展開する.
 2. ユーザごとに設定ファイルを変えたい場合は, z1f9configをC:\Users\<ユーザ名>
    にコピーする.
    全ユーザで共通としたい場合は, z1f9.exeと同じディレクトリのままにする.
    Windowsの場合はz1f9configが存在するディレクトリがこのソフトウェアのホーム
    ディレクトリ(~\)となる.
 3. ROMイメージファイル(rom.txt)をz1configで設定したディレクトリ(デフォルトは
    z1configと同じディレクトリ)にコピーする.


* 使用方法 *
 ・起動
 z1f9の実行ファイルのアイコンをダブルクリックすると起動する.

 マシン語のファイルをアイコンにドラッグ&ドロップする, または実行ファイルのパラ
 メータとすると, そのファイルを読み込んだ後に実行する.
 ロードするアドレスと実行開始アドレスはヘッダに従う.
 ヘッダがない場合は2000Hにロードし, 2000Hから実行する.

 マシン語のファイルの後にアドレスを16進数で指定すると, 読込後にそのアドレスか
 ら実行する.
 例: z1f9 prog.bin 3000

 実行ファイルのパラメータに -<項目>=<値> を付けると, 設定ファイルの設定を上書
 きする.
 例: z1f9 -zoom=6

 ・仮想フロッピーディスク
 設定ファイルで指定したディレクトリ内のファイルをフロッピーディスクのファイル
 としてアクセスすることができる.

 仮想フロッピーディスクで扱えるファイルは, ファイル名8文字以内, 拡張子3文字以
 内, ASCII文字と半角カナのみで構成されるもののみである. それ以外のファイルは
 存在しないものとして扱われる.
 さらに, 大文字と小文字を区別するファイルシステム(LinuxやMacOSX)の場合, 小文
 字が含まれるファイルも扱えない.

 ウィンドウにディレクトリをドラッグ&ドロップするとそのディレクトリが, ファイ
 ルをドラッグ&ドロップするとそのファイルのあるディレクトリが, 仮想フロッピー
 ディスクとなる.

 仮想フロッピーディスクのファイルをKILLすると, 実際に削除されるので注意するこ
 と. なお, FORMATは何も行わない.

 ・自動入力
 テキストをクリップボードにコピーし, エミュレータ側でマウスの中ボタンをクリッ
 クする, またはCTRL+Vキー(デフォルト)を押すと, クリップボードの内容を自動入力
 する.
 このとき全角カタカナ, 全角ひらがなは半角カナに変換される.


* 設定 *
 z1f9config(または.z1f9config)を編集すると設定を変えることができる.
 下の説明の|はいずれか1つを選択すること, <...>は適当な数値や文字列を表す.

 Windowsの場合, 設定ファイルにBOMがあればUTF-8, なければその環境の文字コード
 (日本語環境ならばShiftJIS)として扱われる.
 Windows以外はその環境のロケールに従う.

machine    z1|z1gr|z1gra|fx890p|fx890p_en
    エミュレートする機種を指定する.

rom_path   <パス名>
    ROMイメージのフルパス名を指定する.
    (空欄の場合, BIOSコールをエミュレートするが, ほとんどの機能が実装されていな
    い.)

ram_path   <パス名>
    RAMイメージのフルパス名を指定する.

clock      <クロック周波数>
    CPUのクロック周波数をHz単位で指定する.

ram_size   <RAM容量>
    RAM容量をKB単位で指定する. 最大値は256である.

zoom       <LCD倍率>
    LCDの表示の倍率を指定する.

lcd_scales <LCD階調数>
    液晶の残像シミュレートの階調数を指定する.
    2のとき残像をシミュレートしない.
    0のとき最大限シミュレートする.

refresh    <I/O更新周期>
    I/Oの更新周期をHz単位で指定する.

buzzer     y|n
    ブザー音の出力を設定する.
    yのとき出力する. nのとき出力しない.

floppy_dir <ディレクトリ名>
    仮想フロッピーディスクのディレクトリ名を指定する.

sio_in     <パス名>
    シリアル入力の内容が書き込まれたファイルのフルパス名を指定する.

sio_out    <パス名>
    シリアル出力の内容が書き込まれるファイルのフルパス名を指定する.

printer    <パス名>
    プリンタ出力の内容が書き込まれるファイルのフルパス名を指定する.

<z1f9key>  <key>
    ポケコンのキー<z1f9key>にエミュレートする側のキー<key>を割り当てる.
    <key>の前に&をつけるとALT, ^を付けるとCTRL, +を付けるとSHIFTとのコンビネ
    ーションとなる.
    キー名は付録を参照すること.
    エミュレートする側のキーは, キーに割り当てられた文字ではなく, 106/109日本
    語キーボードにおける物理的な位置を表しているので注意すること.
    例えば, ASCII配列キーボードを使っていて{キーを割り当てたい場合, このキー
    の位置にある日本語キーボードのキーは@なので, @を指定しなければならない.

joy_up      <z1f9key>
joy_down    <z1f9key>
joy_left    <z1f9key>
joy_right   <z1f9key>
joy_button1 <z1f9key>
joy_button2 <z1f9key>
joy_button3 <z1f9key>
joy_button4 <z1f9key>
joy_button5 <z1f9key>
joy_button6 <z1f9key>
joy_button7 <z1f9key>
joy_button8 <z1f9key>
joy_button9 <z1f9key>
    ジョイスティックにポケコンのキーを割り当てる.


* 注意 *
 ・はじめて起動した場合は, オールリセットする必要がある.


* 付録 *
 ・ポケコン側のキー名 一覧
 通常キー
 a b c d e f g h i j k l m n o p q r s t u v w x y z ; : , = spc
 0 1 2 3 4 5 6 7 8 9 . exponent + - * / return
 brk bs shift 2nd
 tab caps calc spc ins left down up del right
 ins del tab caps right left up down
 srch in out calc
 menu cal sqr x^2 eng cls log ln degr sin cos tan mr m+ ( ) ^ ans
 allreset

 コンビネーションキー
 ! " hash $ % & , \ | , @ = ? { } [ ] < > _ angle print system clear cont
 renum run edit list delete
 p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 ran pi pol rec npr cnr
 lcan
 kana ltop lend ftop fend
 line
 submenu cur x^3 seng home 10^x 10^ exp dms asn acs atn min m- &h hex fact set

 仮想キー
 off copy paste rewind_inport rewind_outport

 ・エミュレートする側のキー名 一覧
 backspace tab clear return enter pause escape space : , - . / ; ^ @ [ ] `
 0 1 2 3 4 5 6 7 8 9
 n0 n1 n2 n3 n4 n5 n6 n7 n8 n9 n. n+ n- n* n/ nenter n=    ※テンキーのキー
 a b c d e f g h i j k l m n o p q r s t u v w x y z
 delete up down left right insert home end pageup pagedown
 f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15
 numlock capslock scrolllock
 lshift rshift lctrl rctrl lalt ralt
 mode compose help print sysreq break menu power
 _ kana \ xfer nfer    ※106/109日本語キーボードのキー
 non-us-backslash      ※ドイツ語等キーボードの左シフトキーの右隣のキー


* ライセンス *

 ROMイメージがない場合, 門真なむさんの6×8 ドット日本語フォント「k6x8」の一部
 文字を使用しています. ライセンスは次の通りです.

 --- k6x8のライセンス ここから ---
 These fonts are free software.
Unlimited permission is granted to use, copy, and distribute them, with or without modification, either commercially or noncommercially.
THESE FONTS ARE PROVIDED "AS IS" WITHOUT WARRANTY.

これらのフォントはフリー（自由な）ソフトウエアです。
あらゆる改変の有無に関わらず、また商業的な利用であっても、自由にご利用、複製、再配布することができますが、全て無保証とさせていただきます。 
 --- k6x8のライセンス ここまで ---


 z1f9はBSDライセンスです.

 Copyright (c) 2009 ~ 2020 maruhiro
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
