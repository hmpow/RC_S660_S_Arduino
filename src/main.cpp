#include <Arduino.h>
#include "./rcs660s/rcs660s_apdu.h"

//マニュアル指定定数
const uint8_t FULL_COMMAND_GetFirmWareVersion[] = {0x00, 0x00, 0xFF, 0x00, 0x0E, 0xF2, 0x6B, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x56, 0x00, 0x00, 0x3C, 0x00};


void setup() {
  // put your setup code here, to run once:
  setupSerial();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //レシーバ初期化
  uart_receiver_init();

  delay(2000);

  //rcs660 に GetFirmwareVersionを送信
  debugPrintMsg("\n--TX GetFirmwareVersion--");
  assemblyAPDUcommand_GetFirmwareVersion();

  //ACK確認
  debugPrintMsg("\n--RX ACK--");
  (void)uart_receiver_checkACK();

  //データ受信
  debugPrintMsg("\n--RX DATA--");
  uint8_t sprittedDataArr[RECEIVE_DATA_BUFF_SIZE];
  uint16_t sprittedDataLen = 0;
  bool isReceived = false;

  isReceived = uart_receiver_receiveData(sprittedDataArr, &sprittedDataLen); 

  if(isReceived){
    debugPrintMsg("\nRX SUCCESS\nDATA = ");
    for (size_t i = 0; i < sprittedDataLen; i++)
    {
      debugPrintHex(sprittedDataArr[i]);
    }
    isReceived = false;
  }else{
    debugPrintMsg("RX NO DATA");
  }
  debugPrintMsg("\n-----\n"); 
}
