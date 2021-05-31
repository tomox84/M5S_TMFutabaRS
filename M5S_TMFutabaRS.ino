#include "TMFutabaRS.h"
#include <M5Stack.h>

// Serial2(M5StackはSerial1が使えない)
const int PinRX = 16; // From BTE094 TXD (Blue)
const int PinTX = 17; // From BTE094 RXD (GREEN)

// サーボの定義
TMFutabaRS servo1(1);
TMFutabaRS servo2(2);
TMFutabaRS servo3(3);
TMFutabaRS servo4(4);
TMFutabaRS servo5(5);

void setup() {
  M5.begin();
  M5.Power.begin();

  // サーボ制御用
  Serial2.begin(115200, SERIAL_8N1, PinRX, PinTX);

  // サーボにサーボ制御用シリアルをセット
  servo1.begin(&Serial2);
  servo2.begin(&Serial2);
  servo3.begin(&Serial2);
  servo4.begin(&Serial2);
  servo5.begin(&Serial2);

  // トップメッセージ
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN);

  M5.Lcd.fillScreen(BLUE);

  // メモリマップの読み込み
  if (servo1.MemoryMap00to29()) {
    DispMap00to29(&servo1);
    DispSerialMap00to29(&servo1);
  }
  else {
    // 失敗時
    M5.Lcd.println("Cannot read map 00-29");
    Serial.println("Cannot read map 00-29");
  }

  if (servo1.MemoryMap30to59()) {
    // 情報表示
    DispSerialMap30to59(&servo1);
  }
  else {
    // 失敗時
    Serial.println("Cannot read map 30-59");
  }

  if (servo1.MemoryMap42to59()) {
    // 情報表示
    DispSerialMap42to59(&servo1);
  }
  else {
    // 失敗時
    Serial.println("Cannot read map 42-59");
  }


  // ボタンの何かを押すまで待機
  while ((M5.BtnA.wasPressed() == false) && (M5.BtnB.wasPressed() == false) && (M5.BtnC.wasPressed() == false)) {
    M5.update();
    delay(50);
  }
  M5.update();

  // バックの色
  M5.Lcd.fillScreen(BLUE);
  // 文字の色
  M5.Lcd.setTextColor(WHITE, BLUE);

  // トルクオン
  servo1.TorqueOn();
  servo2.TorqueOn();
  servo3.TorqueOn();
  servo4.TorqueOn();
  servo5.TorqueOn();

  // 0度へ移動
  servo1.Move(0, 10);
  servo2.Move(0, 10);
  servo3.Move(0, 10);
  servo4.Move(0, 10);
  servo5.Move(0, 10);

  DispBack();

//    servo1.ChangeID( 6, &Serial);
  //  servo1.ResetFactoryDefault(&Serial);
}

// Max
float maxPos1 = 0.0;
float maxPos2 = 0.0;
float maxPos3 = 0.0;
float maxPos4 = 0.0;
float maxPos5 = 0.0;
int maxCur1 = 0;
int maxCur2 = 0;
int maxCur3 = 0;
int maxCur4 = 0;
int maxCur5 = 0;

void clearMax(void) {
  maxPos1 = 0.0;
  maxPos2 = 0.0;
  maxPos3 = 0.0;
  maxPos4 = 0.0;
  maxPos5 = 0.0;
  maxCur1 = 0;
  maxCur2 = 0;
  maxCur3 = 0;
  maxCur4 = 0;
  maxCur5 = 0;
}


const int px1 = 10;
const int px2 = 100;
const int py = 0;

int cnt = 0;
void loop() {

  // --Servo1 情報表示
  M5.Lcd.setCursor(px2, py + 20);
  if (servo1.MemoryMap42to59()) {
    float pos = (float)(servo1.CurrentPosition) * 0.1;
    int cur = servo1.CurrentCurrent;
    M5.Lcd.printf("%5.1fdeg  %3dmA", pos, cur);
    if (maxCur1 < cur) {
      maxCur1 = cur;
      maxPos1 = pos;
    }
  }
  else {
    M5.Lcd.print("N/A            ");
  }

  // --Servo2 情報表示
  M5.Lcd.setCursor(px2, py + 40);
  if (servo2.MemoryMap42to59()) {
    float pos = (float)(servo2.CurrentPosition) * 0.1;
    int cur = servo2.CurrentCurrent;
    M5.Lcd.printf("%5.1fdeg  %3dmA", pos, cur);
    if (maxCur2 < cur) {
      maxCur2 = cur;
      maxPos2 = pos;
    }
  }
  else {
    M5.Lcd.print("N/A            ");
  }

  // --Servo3 情報表示
  M5.Lcd.setCursor(px2, py + 60);
  if (servo3.MemoryMap42to59()) {
    float pos = (float)(servo3.CurrentPosition) * 0.1;
    int cur = servo3.CurrentCurrent;
    M5.Lcd.printf("%5.1fdeg  %3dmA", pos, cur);
    if (maxCur3 < cur) {
      maxCur3 = cur;
      maxPos3 = pos;
    }
  }
  else {
    M5.Lcd.print("N/A            ");
  }

  // --Servo4 情報表示
  M5.Lcd.setCursor(px2, py + 80);
  if (servo4.MemoryMap42to59()) {
    float pos = (float)(servo4.CurrentPosition) * 0.1;
    int cur = servo4.CurrentCurrent;
    M5.Lcd.printf("%5.1fdeg  %3dmA", pos, cur);
    if (maxCur4 < cur) {
      maxCur4 = cur;
      maxPos4 = pos;
    }
  }
  else {
    M5.Lcd.print("N/A            ");
  }

  // --Servo5 情報表示
  M5.Lcd.setCursor(px2, py + 100);
  if (servo5.MemoryMap42to59()) {
    float pos = (float)(servo5.CurrentPosition) * 0.1;
    int cur = servo5.CurrentCurrent;
    M5.Lcd.printf("%5.1fdeg  %3dmA", pos, cur);
    if (maxCur5 < cur) {
      maxCur5 = cur;
      maxPos5 = pos;
    }
  }
  else {
    M5.Lcd.print("N/A            ");
  }

  // Max
  DispMax();

  // ボタン
  if (M5.BtnA.wasPressed()) {
    clearMax();
    servo1.Pos(1000);
    servo2.Pos(0);
    servo3.Pos(500);
    servo4.Pos(-400);
    servo5.Pos(800);
  }
  if (M5.BtnB.wasPressed()) {
    clearMax();
    servo1.Pos(100);
    servo2.Pos(1000);
    servo3.Pos(-500);
    servo4.Pos(500);
    servo5.Pos(-800);
  }
  if (M5.BtnC.wasPressed()) {
    M5.Lcd.fillScreen(YELLOW);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println(servo1.Ack() ? "ACK -> OK" :  "ACK -> NG");
    M5.Lcd.println("");
    M5.Lcd.setTextColor(BLACK);
    if (servo1.MemoryMap30to59()) {
      // 情報表示
      DispSerialMap30to59(&servo1);
      DispMap30to59(&servo1);
    }
    else {
      // 失敗時
      M5.Lcd.println("Cannot read memory map 30-59");
    }

    cnt++;
    if ((cnt % 2) == 0) {
      servo1.MoveTime(0);
      servo2.MoveTime(0);
      servo3.MoveTime(0);
      servo4.MoveTime(0);
      servo5.MoveTime(0);
    }
    else {
      servo1.MoveTime(200);
      servo2.MoveTime(200);
      servo3.MoveTime(200);
      servo4.MoveTime(200);
      servo5.MoveTime(200);
    }

    // ボタンが押されるまで待機
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("-- Press Right button --");
    M5.update();
    while (M5.BtnC.wasPressed() == false) {
      M5.update();
      delay(50);
    }
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextColor(WHITE, BLUE);
    DispBack();
  }

  M5.update();
  delay(50);

}

/*-------------------------------------------------
  名前: 情報表示のバックの表示
  機能: DispBack
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void DispBack(void) {
  M5.Lcd.setCursor(px1, py + 0);
  M5.Lcd.print("-- Current --");
  M5.Lcd.setCursor(px1, py + 20);
  M5.Lcd.print("Servo1");
  M5.Lcd.setCursor(px1, py + 40);
  M5.Lcd.print("Servo2");
  M5.Lcd.setCursor(px1, py + 60);
  M5.Lcd.print("Servo3");
  M5.Lcd.setCursor(px1, py + 80);
  M5.Lcd.print("Servo4");
  M5.Lcd.setCursor(px1, py + 100);
  M5.Lcd.print("Servo5");

  M5.Lcd.setCursor(px1, py + 120);
  M5.Lcd.print("-- Max mA --");
  M5.Lcd.setCursor(px1, py + 140);
  M5.Lcd.print("Servo1");
  M5.Lcd.setCursor(px1, py + 160);
  M5.Lcd.print("Servo2");
  M5.Lcd.setCursor(px1, py + 180);
  M5.Lcd.print("Servo3");
  M5.Lcd.setCursor(px1, py + 200);
  M5.Lcd.print("Servo4");
  M5.Lcd.setCursor(px1, py + 220);
  M5.Lcd.print("Servo5");
}

/*-------------------------------------------------
  名前: 動作の最大電流時の角度、電流表示
  機能: DispMax
  引数: なし
  戻値: なし
  -------------------------------------------------*/
void DispMax(void) {
  // Max
  M5.Lcd.setCursor(px2, py + 140);
  M5.Lcd.printf("%5.1fdeg  %3dmA", maxPos1, maxCur1);
  M5.Lcd.setCursor(px2, py + 160);
  M5.Lcd.printf("%5.1fdeg  %3dmA", maxPos2, maxCur2);
  M5.Lcd.setCursor(px2, py + 180);
  M5.Lcd.printf("%5.1fdeg  %3dmA", maxPos3, maxCur3);
  M5.Lcd.setCursor(px2, py + 200);
  M5.Lcd.printf("%5.1fdeg  %3dmA", maxPos4, maxCur4);
  M5.Lcd.setCursor(px2, py + 220);
  M5.Lcd.printf("%5.1fdeg  %3dmA", maxPos5, maxCur5);
}

/*-------------------------------------------------
  名前: メモリマップの表示
  機能: DispSerialMap00to29
  引数
    srv: 表示するサーボ
  戻値: なし
  -------------------------------------------------*/
void DispMap00to29(TMFutabaRS *srv) {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Model:%s ver.%02X ID:%02X\n", srv->ModelString(), srv->FirmwareVersion, srv->ID);
  M5.Lcd.printf("Reverse        %s\n", srv->isReverse ? "false" : "true");
  M5.Lcd.printf("Baud Rate      %d bps\n", srv->BaudRate());
  M5.Lcd.printf("Return Delay   %d us\n", srv->ReturnDelayMicroSec);
  M5.Lcd.printf("Limit Angle R  %-+3.1f deg\n", (float)(srv->LimitAngleR) * 0.1);
  M5.Lcd.printf("Limit Angle L  %-+3.1f deg\n", (float)(srv->LimitAngleL) * 0.1);
  M5.Lcd.printf("Temp. Limit    %d deg\n", srv->LimitTemp);
  M5.Lcd.printf("Torque Silence %s\n", srv->TorqueInSilenceString());
  M5.Lcd.printf("Warm-up Time   %1.2f s\n", (float)(srv->WarmUpTime) * 0.01);
  M5.Lcd.printf("Compli MarginR %2.1f deg\n", (float)(srv->ComplianceMarginR) * 0.1);
  M5.Lcd.printf("Compli MarginL %2.1f deg\n", (float)(srv->ComplianceMarginL) * 0.1);
  M5.Lcd.printf("Compli Slope R %d deg\n", srv->ComplianceSlopeR);
  M5.Lcd.printf("Compli Slope L %d deg\n", srv->ComplianceSlopeL);
  M5.Lcd.printf("Punch          %3.2f %%\n", (float)(srv->Punch) * 0.01);
}

void DispMap30to59(TMFutabaRS *srv) {
  M5.Lcd.printf("Goal Position: %3.1f deg\n", (float)(srv->GoalPosition) * 0.1);
  M5.Lcd.printf("Goal Time    : %d ms\n", srv->GoalTime);
  M5.Lcd.printf("Max Torque   : %d %%\n", srv->MaxTorque);
  M5.Lcd.printf("Torque Mode  : %s\n", srv->TorqueModeString());
  M5.Lcd.println("");
  M5.Lcd.println("-- Current --");
  M5.Lcd.printf("Position     : %3.1f deg\n", (float)(srv->CurrentPosition) * 0.1);
  M5.Lcd.printf("Time         : %d ms\n", srv->CurrentTime);
  M5.Lcd.printf("Speed        : %d deg/s\n", srv->CurrentSpeed);
  M5.Lcd.printf("Current      : %d mA\n", srv->CurrentCurrent);
  M5.Lcd.printf("Temperature  : %d deg\n", srv->CurrentTemperature);
  M5.Lcd.printf("Volts        : %1.2f v\n", servo1.CurrentVolts * 0.01);
}


/*-------------------------------------------------
  名前: メモリマップの表示
  機能: DispSerialMap00to29
  引数
    srv: 表示するサーボ
  戻値: なし
  -------------------------------------------------*/
void DispSerialMap00to29(TMFutabaRS *srv) {
  Serial.println("-- Memory map 00-29 ---------------");
  Serial.printf(" Model                  : %s\n", srv->ModelString());
  Serial.printf(" Firmware Version       : %02X\n", srv->FirmwareVersion);
  Serial.printf(" ID                     : %02X\n", srv->ID);
  Serial.printf(" Reverse                : %s\n", srv->isReverse ? "false" : "true");
  Serial.printf(" Baud Rate              : %d bps\n", srv->BaudRate());
  Serial.printf(" Return Delay           : %d usec\n", srv->ReturnDelayMicroSec);
  Serial.printf(" Right Limit Angle      : %-+3.1f deg\n", (float)(srv->LimitAngleR) * 0.1);
  Serial.printf(" Left  Limit Angle      : %-+3.1f deg\n", (float)(srv->LimitAngleL) * 0.1);
  Serial.printf(" Temperature Limit      : %d deg\n", srv->LimitTemp);
  Serial.printf(" Torque In Silence      : %s\n", srv->TorqueInSilenceString());
  Serial.printf(" Warm-up Time           : %1.2f sec\n", (float)(srv->WarmUpTime) * 0.01);
  Serial.printf(" Right Compliance Margin: %2.1f deg\n", (float)(srv->ComplianceMarginR) * 0.1);
  Serial.printf(" Left  Compliance Margin: %2.1f deg\n", (float)(srv->ComplianceMarginL) * 0.1);
  Serial.printf(" Right Compliance Slope : %d deg\n", srv->ComplianceSlopeR);
  Serial.printf(" Left  Compliance Slope : %d deg\n", srv->ComplianceSlopeL);
  Serial.printf(" Punch                  : %3.2f %%\n", (float)(srv->Punch) * 0.01);
  Serial.println("-----------------------------------");
}

/*-------------------------------------------------
  名前: メモリマップの表示
  機能: DispSerialMap30to59
  引数
    srv: 表示するサーボ
  戻値: なし
  -------------------------------------------------*/
void DispSerialMap30to59(TMFutabaRS *srv) {
  Serial.println("-- Memory map 30-59 ---------------");
  Serial.printf(" Goal Position          : %3.1f deg\n", (float)(srv->GoalPosition) * 0.1);
  Serial.printf(" Goal Time              : %d msec\n", srv->GoalTime);
  Serial.printf(" Max Torque             : %d %%\n", srv->MaxTorque);
  Serial.printf(" Torque Mode            : %s\n", srv->TorqueModeString());
  Serial.printf(" Current Position       : %3.1f deg\n", (float)(srv->CurrentPosition) * 0.1);
  Serial.printf(" Current Time           : %d msec\n", srv->CurrentTime * 10);
  Serial.printf(" Current Speed          : %d deg/sec\n", srv->CurrentSpeed);
  Serial.printf(" Current Current        : %d mA\n", srv->CurrentCurrent);
  Serial.printf(" Current Temperature    : %d deg\n", srv->CurrentTemperature);
  Serial.printf(" Current Volts          : %1.2f V\n", srv->CurrentVolts * 0.01);
  Serial.println("-----------------------------------");
}

/*-------------------------------------------------
  名前: メモリマップの表示
  機能: DispSerialMap42to59
  引数
  srv: 表示するサーボ
  戻値: なし
  -------------------------------------------------*/
void DispSerialMap42to59(TMFutabaRS *srv) {
  Serial.println("-- Memory map 42-59 ---------------");
  Serial.printf(" Current Position       : %3.1f deg\n", (float)(srv->CurrentPosition) * 0.1);
  Serial.printf(" Current Time           : %d msec\n", srv->CurrentTime);
  Serial.printf(" Current Speed          : %d deg/sec\n", srv->CurrentSpeed);
  Serial.printf(" Current Current        : %d mA\n", srv->CurrentCurrent);
  Serial.printf(" Current Temperature    : %d deg\n", srv->CurrentTemperature);
  Serial.printf(" Current Volts          : %1.2f V\n", srv->CurrentVolts * 0.01);
  Serial.println("-----------------------------------");
}
