#include "TMFutabaRS.h"
// ---------------------------------------
// TMFutabaRS クラス
// ver. 1.0 2021-05-19
// 
// Futabaのサーボモータ　コマンド制御クラス
// ---------------------------------------

// -- コンストラクタ
/*-------------------------------------------------
  機能: コンストラクタ
  引数
    Id: サーボID
  -------------------------------------------------*/
TMFutabaRS::TMFutabaRS(unsigned char Id) {
  _Id = Id;
}

/*-------------------------------------------------
  名前: begin
  機能: 使用開始
  引数
    SerialServo: 制御用シリアル
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::begin(HardwareSerial* SerialServo) {
  _SerialServo = SerialServo;
}

// -- 送信
/*-------------------------------------------------
  名前: SendBin
  機能: バイナリをTXDに送信する
  引数:
    txData: TXDに送信するバイナリデータ
    sizeData: TXDに送信するバイナリデータのサイズ
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::SendBin(unsigned char* txData, size_t sizeData) {
  _SerialServo->write(txData, sizeData);
  _SerialServo->flush();        // データ送信完了待ち
}

/*-------------------------------------------------
  名前: rxReadByte
  機能: 1文字(1Byte)受信する
  引数
    readData: 読み込んだデータを格納する
    countMax: リトライする最大カウント
    waitMs: リトライのループのウェイト
  戻値
    true: 成功
    false: タイムアウト
  -------------------------------------------------*/
boolean TMFutabaRS::rxReadByte(unsigned char* readData, int countMax, int waitMs) {
  int cnt = 0;
  while (_SerialServo->available() == 0) {
    cnt++;
    if (cnt >= countMax) return false;
    delay(waitMs);
  }
  *readData = _SerialServo->read();
  return true;
}

/*-------------------------------------------------
  名前: rxReadByte
  機能: 1文字(1Byte)受信する
  引数
    readData: 読み込んだデータを格納する
  戻値
    true: 成功
    false: タイムアウト
  -------------------------------------------------*/
boolean TMFutabaRS::rxReadByte(unsigned char* readData) {
  return rxReadByte(readData, _cntMax, _loopWaitMs);
}

/*-------------------------------------------------
  名前: rxCompare
  機能: rxから読み込んだ1文字(1Byte)を比較する
  引数
    compData: 比較する文字
  戻値
    true: 一致
    false: 不一致またはタイムアウト
  -------------------------------------------------*/
boolean TMFutabaRS::rxCompare(unsigned char compData) {
  unsigned char readData;
  if(rxReadByte(&readData)==false) return false;

  return readData == compData;
}

/*-------------------------------------------------
  名前: rxRead
  機能: 複数データを受信する
  引数
    readData: 読み込んだデータを格納する配列ポインタ
    sizeData: 読み込むデータ数
  戻値
    true: 成功
    false: タイムアウト
  -------------------------------------------------*/
boolean TMFutabaRS::rxRead(unsigned char* readData, size_t sizeData) {
  unsigned char inData;
  for (int i = 0; i < sizeData; i++) {
    if (rxReadByte(&inData, _cntMax, _loopWaitMs) == false) return false;
    readData[i] = inData;
  }
  // 残りのデータはクリアする
  rxClear();
  return true;
}

/*-------------------------------------------------
  名前: rxRead
  機能: 複数データを受信する
  引数
    readData: 読み込んだデータを格納する配列ポインタ
    sizeData: 読み込むデータ数
    countMax: リトライする最大カウント
    waitMs: リトライのループのウェイト
    
  戻値
    true: 成功
    false: タイムアウト
  -------------------------------------------------*/
boolean TMFutabaRS::rxRead(unsigned char* readData, size_t sizeData, int countMax, int waitMs) {
  unsigned char inData;
  for (int i = 0; i < sizeData; i++) {
    if (rxReadByte(&inData, countMax, waitMs) == false) return false;
    readData[i] = inData;
  }
  // 残りのデータはクリアする
  rxClear();
  return true;
}

/*-------------------------------------------------
  名前: rxClear
  機能: RXのデータを空読みしてクリアする
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::rxClear(void) {
  while (_SerialServo->available()) {
    _SerialServo->read();
  }
}

// -- コマンド --
/*-------------------------------------------------
  名前: MemoryMap00to29
  機能: メモリマップ00-29の取得の取得
  引数: なし
  戻値
    true: 成功
    false: 失敗
  -------------------------------------------------*/
boolean TMFutabaRS::MemoryMap00to29(void) {
  // 送信データ(8byte)
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x03;           // Flags
  txData[4] = 0x00;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x01;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);

  // データの読み込み
  size_t sizeReadData = 38;
  unsigned char readData[sizeReadData];
  if (rxRead(readData, sizeReadData) == false) return false;

  // チェックサム計算とチェック
  unsigned char ckSum = 0;
  for (int i = 2; i < sizeReadData - 1; i++) {
    ckSum ^= readData[i]; // ID～DATAまでのXOR
  }
  if (ckSum != readData[sizeReadData - 1]) return false;

  // モデルの格納
  ModelNumber = (short int)(readData[8] << 8);   // Hi byte
  ModelNumber |= (short int)readData[7];         // Lo byte

  // ファームウエアバージョン
  FirmwareVersion = readData[9];

  // ID
  ID = readData[11];
  isReverse = (readData[12] == 0x00);

  // Baud Rate
  _BaudRate = readData[13];

  // Return Delay
  ReturnDelayMicroSec = 100 + 50*(int)readData[14];

  // Limit Angle
  LimitAngleR = (short int)(readData[16] << 8);   // Hi byte
  LimitAngleR |= (short int)readData[15];         // Lo byte
  LimitAngleL = (short int)(readData[18] << 8);   // Hi byte
  LimitAngleL |= (short int)readData[17];         // Lo byte

  // Temperature
  LimitTemp = (short int)(readData[22] << 8);   // Hi byte
  LimitTemp |= (short int)readData[21];         // Lo byte

  // Torque In Silence
  TorqueInSilence = readData[29];

  // Warm-up Time
  WarmUpTime = readData[30];

  // Complianc eMargin
  ComplianceMarginR = readData[31];
  ComplianceMarginL = readData[32];
  ComplianceSlopeR = readData[33];
  ComplianceSlopeL = readData[34];

  // Punch
  Punch = (short int)(readData[36] << 8);   // Hi byte
  Punch |= (short int)readData[35];         // Lo byte

  return true;
}

/*-------------------------------------------------
  名前: MemoryMap30to59
  機能: メモリマップ30-59の取得の取得
  引数: なし
  戻値
    true: 成功
    false: 失敗
  -------------------------------------------------*/
boolean TMFutabaRS::MemoryMap30to59(void) {
  // 送信データ(8byte)
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x05;           // Flags
  txData[4] = 0x00;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x01;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);

  // データの読み込み
  size_t sizeReadData = 38;
  unsigned char readData[sizeReadData];
  if (rxRead(readData, sizeReadData) == false) return false;

  // チェックサム計算とチェック
  unsigned char ckSum = 0;
  for (int i = 2; i < sizeReadData - 1; i++) {
    ckSum ^= readData[i]; // ID～DATAまでのXOR
  }
  if (ckSum != readData[sizeReadData - 1]) return false;

  // 指示位置
  GoalPosition = (short int)(readData[8] << 8);   // Hi byte
  GoalPosition |= (short int)readData[7];         // Lo byte
  
  // 指示時間
  GoalTime = (short int)(readData[10] << 8);   // Hi byte
  GoalTime |= (short int)readData[9];         // Lo byte

  // 最大トルク
  MaxTorque = readData[12];
  
  // トルクモード
  TorqueMode = readData[13];

  // 現在位置
  CurrentPosition = (short int)(readData[20] << 8);   // Hi byte
  CurrentPosition |= (short int)readData[19];         // Lo byte

  // 現在時間
  CurrentTime = (short int)(readData[22] << 8);   // Hi byte
  CurrentTime |= (short int)readData[21];         // Lo byte
  
  // 現在スピード
  CurrentSpeed = (short int)(readData[24] << 8);   // Hi byte
  CurrentSpeed |= (short int)readData[23];         // Lo byte

  // 現在負荷
  CurrentCurrent = (short int)(readData[26] << 8);   // Hi byte
  CurrentCurrent |= (short int)readData[25];         // Lo byte

  // 現在温度
  CurrentTemperature = (short int)(readData[28] << 8);   // Hi byte
  CurrentTemperature |= (short int)readData[27];         // Lo byte

  // 現在電圧
  CurrentVolts = (short int)(readData[30] << 8);   // Hi byte
  CurrentVolts |= (short int)readData[29];         // Lo byte

  return true;
}

/*-------------------------------------------------
  名前: MemoryMap42to59
  機能: メモリマップ42-59の取得の取得
  引数
    countMax: リトライする最大カウント
    waitMs: リトライのループのウェイト
  戻値
    true: 成功
    false: 失敗
  -------------------------------------------------*/
boolean TMFutabaRS::MemoryMap42to59(int countMax, int waitMs) {
  // 送信データ
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x09;           // Flags
  txData[4] = 0x00;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x01;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);

  // データの読み込み
  size_t sizeReadData = 26;
  unsigned char readData[sizeReadData];
  if (rxRead(readData, sizeReadData, countMax, waitMs) == false) return false;

  // チェックサム計算とチェック
  unsigned char ckSum = 0;
  for (int i = 2; i < sizeReadData - 1; i++) {
    ckSum ^= readData[i]; // ID～DATAまでのXOR
  }
  if (ckSum != readData[sizeReadData - 1]) return false;

  // 現在位置
  PrevPosition = CurrentPosition;
  CurrentPosition = (short int)(readData[8] << 8);   // Hi byte
  CurrentPosition |= (short int)readData[7];         // Lo byte

  // 現在時間
  PrevTime = CurrentTime;
  CurrentTime = (short int)(readData[10] << 8);   // Hi byte
  CurrentTime |= (short int)readData[9];         // Lo byte
  
  // 現在スピード
  PrevSpeed = CurrentSpeed;
  CurrentSpeed = (short int)(readData[12] << 8);   // Hi byte
  CurrentSpeed |= (short int)readData[11];         // Lo byte

  // 現在負荷
  PrevCurrent = CurrentCurrent;
  CurrentCurrent = (short int)(readData[14] << 8);   // Hi byte
  CurrentCurrent |= (short int)readData[13];         // Lo byte

  // 現在温度
  PrevTemperature = CurrentTemperature;
  CurrentTemperature = (short int)(readData[16] << 8);   // Hi byte
  CurrentTemperature |= (short int)readData[15];         // Lo byte

  // 現在電圧
  PrevVolts = CurrentVolts;
  CurrentVolts = (short int)(readData[18] << 8);   // Hi byte
  CurrentVolts |= (short int)readData[17];         // Lo byte

  return true;
}

/*-------------------------------------------------
  名前: MemoryMap42to59
  機能: メモリマップ42-59の取得の取得
  引数: なし
  戻値
    true: 成功
    false: 失敗
  -------------------------------------------------*/
boolean TMFutabaRS::MemoryMap42to59(void) {
  return MemoryMap42to59(_cntMax, _loopWaitMs);
}

/*-------------------------------------------------
  名前: DiffPosition
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffPosition(void) {
  short int diff = CurrentPosition - PrevPosition;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: DiffTime
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffTime(void) {
  short int diff = CurrentTime - PrevTime;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: DiffSpeed
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffSpeed(void) {
  short int diff = CurrentSpeed - PrevSpeed;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: DiffCurrent
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffCurrent(void) {
  short int diff = CurrentCurrent - PrevCurrent;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: DiffTemperature
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffTemperature(void) {
  short int diff = CurrentTemperature - PrevTemperature;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: DiffVolts
  機能: CurrentとPrevの差（絶対値）
  引数: なし
  戻値: 前回値と今回値の差を絶対値で返す
  -------------------------------------------------*/
short int TMFutabaRS::DiffVolts(void) {
  short int diff = CurrentVolts - PrevVolts;
  return diff >= 0 ? diff : -diff;
}

/*-------------------------------------------------
  名前: ModelString
  機能: Memory_Mapで読み込んだモデル名
  引数: なし
  戻値: モデル名を返す
  -------------------------------------------------*/
String TMFutabaRS::ModelString(void) {
  switch (ModelNumber) {
    case 0x3030: return "RS303MR";
    case 0x3040: return "RS304MD";
  }
  return "Unknown";
}

/*-------------------------------------------------
  名前: ModelString
  機能: Memory_Mapで読み込んだモデル名
  引数: なし
  戻値: モデル名を返す
  -------------------------------------------------*/
int TMFutabaRS::BaudRate(void) {
  switch (_BaudRate) {
    case COM9600: return 9600;
    case COM14400: return 14400;
    case COM19200: return 19200;
    case COM28800: return 28800;
    case COM38400: return 38400;
    case COM57600: return 57600;
    case COM76800: return 76800;
    case COM115200: return 115200;
  }
  return 0;
}

/*-------------------------------------------------
  名前: TorqueInSilenceString
  機能: Memory_Mapで読み込んだTorqueInSilenceを文字で返す
  引数: なし
  戻値: TorqueInSilenceに対応する文字
  -------------------------------------------------*/
String TMFutabaRS::TorqueInSilenceString(void) {
  switch (TorqueInSilence) {
    case 0x00: return "Off";
    case 0x01: return "On";
    case 0x02: return "Brake";
  }
  return "Unknown";
}

/*-------------------------------------------------
  名前: TorqueModeString
  機能: Memory_Mapで読み込んだTorqueModeを文字で返す
  引数: なし
  戻値: TorqueModeに対応する文字
  -------------------------------------------------*/
String TMFutabaRS::TorqueModeString(void) {
  switch (TorqueMode) {
    case 0x00: return "Off";
    case 0x01: return "On";
    case 0x02: return "Brake";
  }
  return "Unknown";
}

/*-------------------------------------------------
  名前: Ackの確認
  機能: Ack
  引数: なし
  戻値
    true: ACKを受信した
    false: ACKを受信できなかった
  -------------------------------------------------*/
boolean TMFutabaRS::Ack(void) {
  // 送信データ(8byte)
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x01;           // Flags
  txData[4] = 0x00;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x00;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);

  // 読み込んだデータとAck(0x07)との比較
  return rxCompare(0x07);
}

/*-------------------------------------------------
  名前: writeFlashRom
  機能: RS304MDのROMに現在の状態を書き込む
  引数: なし
  戻値: なし
  -------------------------------------------------*/
// Write Flash ROM
void TMFutabaRS::writeFlashRom(void) {
  // 送信データ(8byte)
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x40;           // Flags
  txData[4] = 0xFF;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x00;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}

/*-------------------------------------------------
  名前: reboot
  機能: RS304MDを再起動する
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::reboot(void) {
  // 送信データ(8byte)
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x20;           // Flags
  txData[4] = 0xFF;           // Address
  txData[5] = 0x00;           // Length
  txData[6] = 0x00;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}

/*-------------------------------------------------
  名前: torque
  機能: トルクをTXDに送信する
  引数:
    OnOff: On(0x01)かOff(0x00)のバイナリデータ
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::torque(unsigned char OnOff) {
  // 送信データ
  const size_t sizeData = 9;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x24;           // Address
  txData[5] = 0x01;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = OnOff;          // Data(Torque)

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}
/*-------------------------------------------------
  名前: TorqueOn
  機能: トルクをOnにする
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::TorqueOn(void) {
  torque(0x01);
}
/*-------------------------------------------------
  名前: TorqueOff
  機能: トルクをOffにする
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::TorqueOff(void) {
  torque(0x00);
}
/*-------------------------------------------------
  名前: TorqueBrake
  機能: トルクをBrakeモードにする
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::TorqueBrake(void) {
  torque(0x02);
}

/*-------------------------------------------------
  名前: Move
  機能: サーボ角度・速度指定
  引数:
    Angle: サーボ角度
    Speed: 速度
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::Move(int pos, int Tms) {
  // 送信データ
  const size_t sizeData = 12;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x1E;           // Address
  txData[5] = 0x04;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = (unsigned char)0x00FF & pos;           // Data(Angle) Low byte
  txData[8] = (unsigned char)0x00FF & (pos >> 8);    // Data(Angle) Hi  byte
  txData[9] = (unsigned char)0x00FF & Tms;           // Data(Speed) Low byte
  txData[10] = (unsigned char)0x00FF & (Tms >> 8);   // Data(Speed) Hi  byte

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}

/*-------------------------------------------------
  名前: Pos
  機能: サーボ移動角度
  引数:
    pos: サーボ移動角度
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::Pos(int pos) {
  // 送信データ(10byte)
  const size_t sizeData = 10;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x1E;           // Address
  txData[5] = 0x02;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = (unsigned char)0x00FF & pos;           // Data(Angle) Low byte
  txData[8] = (unsigned char)0x00FF & (pos >> 8);    // Data(Angle) Hi  byte

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}

/*-------------------------------------------------
  名前: MoveTime
  機能: サーボ移動速度
  引数:
    Spd: サーボ移動速度
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::MoveTime(int Tms) {
  // 送信データ(10byte)
  const size_t sizeData = 10;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x20;           // Address
  txData[5] = 0x02;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = (unsigned char)0x00FF & Tms;           // Data(Speed) Low byte
  txData[8] = (unsigned char)0x00FF & (Tms >> 8);    // Data(Speed) Hi  byte

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
}

/*-------------------------------------------------
  名前: 通信速度の変更
  機能: ChangeBaudRate
  引数
     ComSpeed: 通信速度 (COM9600,,,COM115200など)
     SerialOut: 表示用Serial
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::ChangeBaudRate(unsigned char ComSpeed, HardwareSerial* SerialOut) {
  // 送信データ
  const size_t sizeData = 9;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x06;           // Address
  txData[5] = 0x01;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = ComSpeed;       // Data(ComSpped)

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
  delay(1000);

  // FlashROMへの書き込み
  writeFlashRom();
  delay(1000);

  // サーボ再起動
  reboot();

  SerialOut->println("*****************************");
  SerialOut->println(" COM Speed changed !!");
  SerialOut->println(" Please reload program");
  SerialOut->println("*****************************");

  while (-1) {
    delay(1000);
  }
}

/*-------------------------------------------------
  名前: 初期状態（工場出荷値）へ戻す
  機能: ResetFactoryDefault
  引数
     SerialOut: 表示用Serial
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::ResetFactoryDefault(HardwareSerial* SerialOut) {
  // 送信データ
  const size_t sizeData = 8;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x10;           // Flags
  txData[4] = 0xFF;           // Address
  txData[5] = 0xFF;           // Length
  txData[6] = 0x00;           // Count

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ送信
  SendBin(txData, sizeData);
  delay(1000);

  // FlashROMへの書き込み
  writeFlashRom();
  delay(1000);

  // サーボ再起動
  reboot();

  SerialOut->println("*****************************");
  SerialOut->println(" Reset factory default !!");
  SerialOut->println(" Please reload program");
  SerialOut->println("*****************************");

  while (-1) {
    delay(1000);
  }
}

/*-------------------------------------------------
  名前: サーボのIDの変更
  機能: ChangeID
  引数
     SerialOut: 表示用Serial
  戻値: なし
  -------------------------------------------------*/
void TMFutabaRS::ChangeID(unsigned char newID, HardwareSerial* SerialOut) {
  // 送信データ
  const size_t sizeData = 9;
  unsigned char txData[sizeData];

  // パケットデータ生成
  txData[0] = 0xFA;           // Header
  txData[1] = 0xAF;           // Header
  txData[2] = _Id;            // ID
  txData[3] = 0x00;           // Flags
  txData[4] = 0x04;           // Address
  txData[5] = 0x01;           // Length
  txData[6] = 0x01;           // Count
  txData[7] = newID;          // New ID

  // チェックサム計算
  txData[sizeData - 1] = 0;
  for (int i = 2; i < sizeData - 1; i++) {
    txData[sizeData - 1] ^= txData[i]; // ID～DATAまでのXOR
  }

  // データ表示
  printData("-- Change ID ------------", txData, sizeData);

  // データ送信
  SendBin(txData, sizeData);
  delay(1000);

  // ID変更
  _Id = newID;

  // FlashROMへの書き込み
  writeFlashRom();
  delay(1000);

  // サーボ再起動
  reboot();

  SerialOut->println("*****************************");
  SerialOut->printf(" Changed ID (%d) !!\n", newID);
  SerialOut->println(" Please reload program");
  SerialOut->println("*****************************");

  while (-1) {
    delay(1000);
  }
}
