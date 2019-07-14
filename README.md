# GPS_MTK333X

Mediatek MTK3333 / MTK3339 GPS Module lightweight class liblary

for Arduino IDE

UART または I2Cのいずれにも対応する。

## 概要

Aruduino 用の GPSインターフェースライブラリとしては TinyGPS / TinyGPS++ が著名だが、
GPS の全機能を網羅するように実装されているため、8bit系 AVR で使うには無駄が多い。
また double 型が使えないため、最小精度が数メートルになってしまう。
このクラスライブラリでは対象 GPSレシーバを MTK3333 / MTK3339 に絞り、
次のような実装とした。

- NMEAデータのうち、RMC（時間情報）と GGA（ロケーション情報）にのみ対応する。
- デバイス応答のうち PMTK001 のみを解釈する。
- float型、double型を使わず、固定小数点整数型で演算する。（浮動小数演算ライブラリをリンクしないで済む・演算精度落ちを防ぐ）
- 時刻情報を BCDカレンダー型で取得できる。（単なるプリントアウト目的なら簡易なコードで済む）
- UART接続、I2C接続のいずれにも対応する。
- UARTでは 9600bps のみの対応とする。

動作確認は
[秋月電子通商 AE-GYSFDMAXB](http://akizukidenshi.com/catalog/g/gK-09991/)
(MTK3339 UART)
および
[SparkFun GPS Breakout XA1110](https://www.sparkfun.com/products/14414)
(MTK3333 I2C)
で行った。

<img src="http://akizukidenshi.com/img/goods/L/K-09991.jpg" width="240">
<img src="https://cdn.sparkfun.com//assets/parts/1/2/3/4/0/14414-02.jpg" width="200">

[MTK3339 Datasheet](http://akizukidenshi.com/download/ds/taiyouyuden/GYSFDMAXB_spec_ae.pdf)
/
[MTK3333 Datasheet](https://s3-ap-southeast-1.amazonaws.com/mediatek-labs-imgs/downloads/d54c0d5ffdffbc55f6c592c6a804d4f2.pdf)
/
[MTK NMEA Packet User Manual](https://www.sparkfun.com/datasheets/GPS/Modules/PMTK_Protocol.pdf)

## Arduino IDE への導入

1. .ZIPアーカイブをダウンロードする。[Click here](https://github.com/askn37/GPS_MTK333X/archive/master.zip)

2. ライブラリマネージャで読み込む

    スケッチ -> ライブラリをインクルード -> .ZIP形式のライブラリをインストール...

3. 依存関係があるライブラリも同様に読み込む

    [Futilities](https://github.com/askn37/Futilities) -- BCDカレンダー時刻型関数ライブラリを含む

## とっかかり

```c
#include <SoftwareSerial>
#include <GPS_MTK333X_SoftwareSerial.h>

#define CONSOLE_BAUD    9600
#define GPS_BAUD        9600

#define GPS_TX		    6
#define GPS_RX		    5

GPS_MTK333X_SoftwareSerial GPS(GPS_RX, GPS_TX);

void setup (void) {
    Serial.begin(CONSOLE_BAUD);
    Serial.println(F("Startup"));

    while (!GPS.begin(GPS_BAUD)) {
        Serial.println(F("GPS notready"));
        delay(1000);
    }

    // RMC & GGA Enable 1Hz output
    GPS.sendMTKcommand(314, F(",0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
}

void loop (void) {
    if (GPS.check() && GPS.isTimeUpdate()) {
        bool f = GPS.isStatusUpdate();
        GPSInfo_t gpsInfo = GPS.value();
        Serial.print(gpsInfo.date, HEX);
        Serial.write(' ');
        Serial.print(gpsInfo.time, HEX);
        Serial.write(' ');
        if (f) {
            Serial.print(gpsInfo.satellites, DEC);
            Serial.write(' ');
            Serial.print(gpsInfo.dop / 100.0);
            Serial.write(' ');
            Serial.print(gpsInfo.latitude / 600000.0, 6);
            Serial.write(' ');
            Serial.print(gpsInfo.longitude / 600000.0, 6);
            Serial.write(' ');
            Serial.print(gpsInfo.altitude / 100.0);
            Serial.write(' ');
            Serial.print(gpsInfo.speed * 0.01852);
            Serial.write(' ');
            Serial.print(gpsInfo.course / 100.0);
            Serial.write(' ');
        }
        Serial.println();
    }
}
```

check() メソッドは呼ばれる都度に受信バッファを読み取り、
NMEA を解析し、1行分のデータが正しく揃うと真を返す。
解析されたデータによって対応する isTimeUpdate() isLocationUpdate()
等が真を返すので目的のデータを専用メソッドで個別に取得するか、
value() メソッドで構造体として一括取得する。

取得されるデータは、時刻については BCDカレンダー型（専用メソッドでは time_t 型も可）、
その他はそれぞれに指定された固定小数点整数として得られる。
人間の目で読めるようにするには例示のように適切な定数で除す必要があるが、
センサーデータとしてサーバに送信する用途なら、
エンドノードからは取得した生データのまま送信して
演算精度の勝るサーバ側で除したほうが欠損もなく、正確かつ有利となる。
（エンドノード側コードも最小限にできる）

# リファレンス

### struct GPSInfo\_t

本クラスが NMEA データを解析して返却可能な情報をまとめた構造体定義。
9項目からなり（現在の仕様では）合計 32byteである。

```c
struct GPSInfo_t{
	bcddate_t date;
	bcdtime_t time;
	int32_t latitude;
	int32_t longitude;
	int32_t altitude;
	uint32_t speed;
	uint16_t course;
	uint16_t satellites;
	uint16_t dop;
	uint16_t millisecond;
};
```

|変数型・メンバー名|意味|精度|備考・例示|
|---|---|---|---|
|bcddate_t date|年月日|BCDカレンダー日付型|0x20170530|
|bcdtime_t time|時分秒|BCDカレンダー時間型|0x235959（22bit幅）|
|int32_t latitude|緯度|1/600000度 正:北緯 負:南緯|最大27bit|
|int32_t longitude|経度|1/600000度 正:東経 負:西経|最大28bit|
|int32_t altitude|高度|1/100メートル|通常使用では20bit幅で足りる|
|uint32_t speed|移動速度|1/100海里毎時|0.01852 を掛けるとキロメートル毎時（正確に）|
|uint16_t course|移動方位|1/100度 真北を0度とし時計回りに360度|最大値:35999|
|uint16_t satellites|追跡衛星数|1以上で時刻、4以上で位置取得可能|最大64（おそらく）|
|uint16_t dop|測位誤差|1/100DOP 大きいほど良好|1.00以上で1/100海里以内？|
|uint16_t millisecond|ミリ秒|1/1000秒|NMEAが生成された時間|

緯度1/60度＝1海里＝1852メートルなので、latitude メンバーの最小測定精度は18.52センチメートルである。

millisecond メンバーは 1Hzを超える情報取得を行わない限り参照する意味は薄いだろう。
dop メンバーも誤差の目安ではあるものの具体的に何メートル上下左右にずれているかまではわからない。
ある閾値以下では複数回取得した結果の平均値を得るといった用途に使うのが良い。

## コンストラクタ

GPS\_MTK333X のコンストラクタには、接続インタフェースに応じて4種類が用意されている。

### GPS\_MTK333X\_SoftwareSerial (uint8\_t RX\_PIN, uint8\_t TX\_PIN)
### GPS\_MTK333X\_MultiUART (uint8\_t RX\_PIN, uint8\_t TX\_PIN)

SoftwareSerial または MultiUART コンストラクタでは、
受信ピン、送信ピンの指定は必須である。
またそれぞれに対応したライブラリを事前にインクルードしていなければならない。
ピン番号は Arudiono で規定されたものとする。
それぞれのピンのIO設定は直ちに行われる。
これを用いた場合の最大ボーレートは 9600bpsである。

```c
#define TX_PIN 6
#define RX_PIN 5

// もっぱらグローバルなオブジェクトとして
#include <SoftwareSerial.h>
GPS_MTK333X_SoftwareSerial GPS(RX_PIN, TX_PIN);
GPS.begin(9600);

// スコープから抜けると破棄されるようなオブジェクトとして
auto GPS = new GPS_MTK333X_SoftwareSerial(RX_PIN, TX_PIN);
GPS->begin(9600);
```

### GPS\_MTK333X\_Serial (void)
### GPS\_MTK333X\_Serial1 (void)
### GPS\_MTK333X\_Serial2 (void)
### GPS\_MTK333X\_Serial4 (void)

HardwareSerial を用いてクラスオブジェクトを作成するコンストラクタ。
9600bps を超えるボーレートを使用する場合は、このコンストラクタが必要である。

```c
GPS_MTK333X_Serial1 GPS(Serial1);
GPS.begin(115200);
```

### GPS\_MTK333X\_I2C (void)

I2Cバスを用いてクラスオブジェクトを作成するコンストラクタ。
クラス名が UART用とは異なる。
また begin() の引数が I2C用に変わる。

```c
GPS_MTK333X_I2C GPS;
GPS.begin(I2C_SPEED_STANDARD);
```

### GPS\_MTK333X\_I2C (uint8\_t intPin)

MTK3333 の INTピンを配線して使用できるのであれば、コンストラクタでそれを指定することもできる。
このピンは I2Cバッファに NMEAデータが用意されると LOWになり、I2Cバッファが空であれば HIGHになる。
これが利用できると I2Cバスを空読みする事がなくなり、負荷が軽くなる。

```c
#define GPS_INT_PIN 4

pinMode(GPS_INT_PIN, INPUT_PULLUP)
GPS_MTK333X_I2C GPS(GPS_INT_PIN);
GPS.begin(I2C_SPEED_STANDARD);
```

## メソッド

### bool begin (long SPEED)

UARTインタフェースを指定のボーレートで初期化する。準備ができれば真を返す。

### bool begin (long I2CSPEED = I2\_SPEED\_FAST, uint8\_t I2CADDR = MT333X\_ADDR)

I2Cバスを第1引数の指定速度で初期化し、第2引数のI2Cアドレスを設定する。準備ができれば真を返す。
引数は両方とも省略でき、その場合は既定値が選ばれる。

### bool check (void)

受信バッファを読み取り、NMEAデータを解析する。1行分のデータが正常に揃えば真を返す。

このメソッドは受信バッファが溢れる前に、継続して繰り返し呼び出す必要がある。
より高速なボーレートを扱う場合は相応の頻度が必要。
（当然だが）割込ルーチン内から呼び出してはならない。

### bool isLineFeed (void)

1行分の正常な NMEAデータが新たに読み取られると真を返す。

### bool isLocationUpdate (void)

RMC または GGA の latitude と longitude が新たに読み取られると真を返す。

### bool isTimeUpdate (void)

RMC の年月日および時分秒が新たに読み取られると真を返す。
GGA の時分秒は無視される。

### bool isStatusUpdate (void)

isLocationUpdate と isTimeUpdate の両方が更新されると真を返す。
換言すれば RMC と GGA が各1行以上正しく読み取られれば真となる。

### void statusReset (void)

以上の4つのフラグをリセットする。
NMEAデータを取得したあと、フラグをリセットすることで
新たなデータが読み取られるまでフラグを偽にしておける。

### time\_t epoch (void)

現在保持している時刻情報を世界協定時（UTC）の unix time epoch に変換し、time_t型で返す。

### uint16\_t millisecond (void)

現在保持している時刻情報のミリ秒を、符号なし整数型で返す。範囲は 0〜999 である。

この項目はリアルタイムの時間を表してはいない。
PMTK220 コマンドで指定した間隔で更新される。

### GPSInfo\_t value (void)

現在保持している以下9つの情報を、GPSInfo_t 構造体でまとめて返す。
同時に statusReset() も実行され、フラグはリセットされる。

### bcddate\_t date (void)

現在保持している時刻情報の年月日を、BCDカレンダー年月日型で返す。

この項目は RMCに現れる。

### bcdtime\_t time (void)

現在保持している時刻情報の時分秒を、BCDカレンダー時分秒型で返す。

この項目は すべての NMEAパケットに現れる。

### int32\_t latitude (void)

現在保持している最新の緯度を、符号付き整数型で返す。

最小精度（1LSB）は 1/600000度である。
なので 600000 で割ると Web地図座標系になる。

この項目は RMCと GGAの両方に現れる。

### int32\_t longitude (void)

現在保持している最新の経度を、符号付き整数型で返す。

最小精度（1LSB）は 1/600000度である。
なので 600000 で割ると Web地図座標系になる。

この項目は RMCと GGAの両方に現れる。

### int32\_t altitude (void)

現在保持している最新の高度を、符号付き整数で返す。

最小精度（1LSB）は 1センチメートルである。
なので 100 で割るとメートルに換算できる。

この項目は GGAに現れる。

### uint32\_t speed (void)

現在保持している最新の移動速度を、符号なし整数で返す。

最小精度（1LSB）は 1/100ノーチカルマイル毎時である。
なので 0.01852 を掛けると、キロメートル毎時に換算できる。
（割算なら 53.99568 ちょうどだが覚えにくい）

本項が "海里" 単位なのは、元になった NMEAフォーマットが海運用途で設計された名残であり、互換性維持のためであろう。

ちなみに "時速1海里" で南北方向に1時間進むと、緯度はちょうど1/60度（1分）増減する。
したがって latitude メンバーはちょうど 10000増減することになる。

この項目は RMCに現れる。

### uint16\_t course (void)

現在保持している最新の移動方位を、符号なし整数で返す。

最小精度（1LSB）は 1/100度である。
真北を0度とし時計回りに360度で1回転する。
従って本項の有効範囲は 0〜35999 である。

この項目は RMCに現れる。

### uint16\_t satellites (void)

現在軌道情報を把握して追跡している GPS/QZSS衛星の数を返す。
MTK333Xの場合は最大64を返す可能性がある。

この項目は GGAに現れる。

### uint16\_t dop (void)

現在のDOP値を、符号なし整数にして返す。

最小精度（1LSB）は 1/100である。

1個以上の衛星を補足していれば、時刻情報は十分校正されている。
（0個なら GPSデバイス内の RTCの更新値が返ってくる）
高さを含む位置情報の算出には最低4個の衛星捕捉が必要である。
それ以上の衛星が視野にある場合は、最も適切な位置にあるだろう4個が選ばれる。
概念的には、うち3個が正三角形を描き、残る1個がその中心に近いほど、DOP値は高くなる。
DOP値が低いということはこの形が歪んでおり、三角測量の精度が悪くなるということである。
衛星捕捉数が多ければ歪んでいない正三角形を見つけやすいので測量精度が上がると換言できる。

この項目は GGAに現れる。

### bool sendMTKcommand (uint16\_t PMTKCODE, String MESSAGE = "")

GPSデバイスに PMTK制御コマンドを送信し、1秒以内に応答があれば、その結果を真偽値で返す。
第1引数にコマンド番号を整数で、第2引数に追加のパラメータを文字列で渡す。

```c
// UARTボーレートを 9600bpsに設定する
GPS.sendMTKcommand(251, F(",9600"));

// 1Hz（1000ms）間隔で NMEAを出力する
GPS.sendMTKcommand(220, F(",1000"));

// AlwaysLocate モード開始
GPS.sendMTKcommand(225, F(",8"));

// 上記を解除してスタンダードモードに遷移
GPS.sendMTKcommand(225, F(",0"));

// QZSS（みちびき）をサポートする
GPS.sendMTKcommand(351, F(",1"));

// RMCとGGAをともに1サイクルで出力する（1サイクル時間は PMTK220 による）
// 各項は ",GLL,RMC,VTG,GGA,GSA,GSV,0,0,0,0,0,0,0,0,0,0,0,ZDA,MCHN"
GPS.sendMTKcommand(314, F(",0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
```

本ライブラリの実装では RMCと GGAしか解釈しないので、
PMTK314でそれら以外を有効にしても、受信バッファとリソースを無駄にするだけである。

#### bool sendUBXcommand (uint16\_t UBXCODE, String MESSAGE = "")

U-blox シリーズの GPSに UBXコマンドを送信する。
使い方は sendMTKcommand() と同様であるが、戻り値はない。（常に偽）

本ライブラリは U-blox シリーズを真にサポートしているわけではないが、
必要最小限の用途には使用に耐えうるので、利便性のためこのコマンドが用意されている。

```c
// GPS（U-blox）出力設定
GPS.sendUBXcommand(40, F(",RMC,1,1,0,0,0,0"));
GPS.sendUBXcommand(40, F(",GGA,1,1,0,0,0,0"));
GPS.sendUBXcommand(40, F(",VTG,1,0,0,0,0,0"));
GPS.sendUBXcommand(40, F(",GLL,1,0,0,0,0,0"));
GPS.sendUBXcommand(40, F(",GSA,1,0,0,0,0,0"));
GPS.sendUBXcommand(40, F(",VSV,1,0,0,0,0,0"));
GPS.flush();
```

#### bool encode (const uint8\_t NMEACHAR)

キャラクタを1文字ずつ渡して NMEAデータを解析するメソッド。
情報が更新されると真を返す。
check() の内部使用メソッド。

## 既知の不具合／制約／課題

- 主要な AVR 以外はテストされていない。
- 古い Arduino IDE には対応しない。1.8.5で動作確認。少なくとも C++11 は使えなければならない。
- 英文マニュアルが未整備である。

## 改版履歴

- 0.1.2
  - インタフェース別コンストラクタとヘッダファイルの整理

- 0.1.1

## 使用許諾

MIT

## 著作表示

朝日薫 / askn
(SenseWay Inc.)
Twitter: [@askn37](https://twitter.com/askn37)
GitHub: https://github.com/askn37
