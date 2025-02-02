#include "rcs660s_apdu.h"


//apduコマンドを配列にしてCCID層にパスする
uint8_t passToCcidLayer(APDU_COMMAND input_apdu_command){

    //使ったシーケンス番号をreturnしたいので最後にインクリメントができない⇒FFスタートで帳尻合わせ
    static uint8_t SeqNo = 0xFF;

    //シーケンス番号をインクリメント
    if(SeqNo < 0xFF){
        SeqNo++;}
    else{
        SeqNo = 0x00;
    }

    //出力用可変長配列を用意

    uint8_t *arrAPDUcommand; //出力apduコマンドフレーム

    uint32_t arrSize = 0;
    if(input_apdu_command.Lc == 0){
        //データ無しコマンドの場合
        arrSize = COMMAND_HEADER_SIZE;
    }else{
        arrSize = COMMAND_HEADER_SIZE + COMMAND_LC_SIZE + input_apdu_command.Lc + COMMAND_LE_SIZE;
    }

    arrAPDUcommand = (uint8_t*)malloc(sizeof(uint8_t) * arrSize);
    
    if(arrAPDUcommand == NULL){
        debugPrintMsg("\nmalloc ERROR!\n");
        exit(1);
    }

    //ヘッダ部を格納
    arrAPDUcommand[0] = input_apdu_command.CLA;
    arrAPDUcommand[1] = input_apdu_command.INS;
    arrAPDUcommand[2] = input_apdu_command.P1;
    arrAPDUcommand[3] = input_apdu_command.P2;

    //データ部を格納
    if(input_apdu_command.Lc > 0){
        //Lc
        arrAPDUcommand[4] = input_apdu_command.Lc;

        //DataIn
        for (uint8_t i = 0; i < input_apdu_command.Lc; i++)
        {
            arrAPDUcommand[i + 5] = input_apdu_command.DataIn[i];
        }
        
        //Le
        arrAPDUcommand[5 + input_apdu_command.Lc] = input_apdu_command.Le;
    }

    debugPrintMsg("\nSTART passToCcidLayer Test Output\n");
    for(int i = 0; i < arrSize; i++){
        debugPrintHex(arrAPDUcommand[i]);
    }
    debugPrintMsg("\nEND passToCcidLayer Test Output\n");

    assemblyCCIDcommand_PC_to_RDR_Escape(arrAPDUcommand, arrSize, SeqNo);

    free(arrAPDUcommand);

    return SeqNo;
}

//4.8章 Manage Session

void assemblyAPDUcommand_ManageSession_StartTransparentSession(void){

    APDU_DATA_OBJECT StartTransparentSession_data_object;

    StartTransparentSession_data_object.Tag    = 0x81; //StartTransparentSession
    StartTransparentSession_data_object.Length = 0x00; //Valueなし
    
    _assemblyAPDUcommand_ManageSession_Base(StartTransparentSession_data_object);

    return;
}

void assemblyAPDUcommand_ManageSession_EndTransparentSession(void){

    APDU_DATA_OBJECT EndTransparentSession_data_object;

    EndTransparentSession_data_object.Tag    = 0x82; //EndTransparentSession
    EndTransparentSession_data_object.Length = 0x00; //Valueなし

    _assemblyAPDUcommand_ManageSession_Base(EndTransparentSession_data_object);

    return;
}

void assemblyAPDUcommand_ManageSession_TrunOffRfField(void){

    APDU_DATA_OBJECT TrunOffRfField_data_object;

    TrunOffRfField_data_object.Tag    = 0x83; //TrunOffRfField
    TrunOffRfField_data_object.Length = 0x00; //Valueなし

    _assemblyAPDUcommand_ManageSession_Base(TrunOffRfField_data_object);

    return;
}

void assemblyAPDUcommand_ManageSession_TrunOnRfField(void){

    APDU_DATA_OBJECT TrunOnRfField_data_object;

    TrunOnRfField_data_object.Tag    = 0x84; //TrunOnRfField
    TrunOnRfField_data_object.Length = 0x00; //Valueなし

    _assemblyAPDUcommand_ManageSession_Base(TrunOnRfField_data_object);

    return;
}

//4.9章 Transparent Exchange

//Transmission and Reception Flagコマンド組み立て(TxCRC付加,RxCRC除去,送受信パリティ,ProtocolProloge自動処理)
void assemblyAPDUcommand_TransparentExchange_TransmissionAndReceptionFlag(
    const bool txDoNotAppendCRC, const bool rcDoNotDiscardCRC, const uint8_t transceiveParity, const bool doNotAppendOrDiscardProcolProloge){

    APDU_DATA_OBJECT transmitAndReceptionFlag_data_object = {0};

    transmitAndReceptionFlag_data_object.Tag    = 0x90; //TransmissionAndReceptionFlag
    transmitAndReceptionFlag_data_object.Length = 0x02; //Value長さ

    //Valueの組み立て
    uint16_t Value = 0x0000;

    Value |= txDoNotAppendCRC << 0;
    Value |= rcDoNotDiscardCRC << 1;
    Value |= (transceiveParity & 0x03) << 2;
    Value |= doNotAppendOrDiscardProcolProloge << 4;
    //5~15 は 0 のまま (RFU)

    transmitAndReceptionFlag_data_object.Value[0] = (uint8_t)((0xFF00 & Value) >> 8);   //上位バイト
    transmitAndReceptionFlag_data_object.Value[1] = (uint8_t)(0x00FF & Value);          //下位バイト

    _assemblyAPDUcommand_TransparentExchange_Base(transmitAndReceptionFlag_data_object, TIMER_NO_USE);

    return;
}


//Transceveコマンド組み立て(無線コマンドarray,無線コマンドlen,受信タイムアウトms)
void assemblyAPDUcommand_Transparent_Exchange_Transceive(const uint8_t WirelessCommand[], const uint8_t WirelessCommand_Len, const uint16_t timeout_ms){

    APDU_DATA_OBJECT transmit_data_object = {0};

    if(WirelessCommand_Len > DATA_OBJECT_VALUE_MAX_SIZE){
        //error(); //データ長が長すぎる
        return;
    }

    //固定値(マニュアル指定値)
    transmit_data_object.Tag = (uint8_t)0x95; //Transceive

    //変数
    transmit_data_object.Length = WirelessCommand_Len;

    for(uint8_t i = 0; i < WirelessCommand_Len; i++){
        transmit_data_object.Value[i] = WirelessCommand[i];
    }

    //Base関数へパス
    _assemblyAPDUcommand_TransparentExchange_Base(transmit_data_object, timeout_ms);
   
    return;
}

//4.10章 Switch Protocol

void assemblyAPDUcommand_SwitchProtocol_TypeA_AutoActivate(void){

    APDU_DATA_OBJECT SwitchProtocol_TypeA_data_object = {0};

    //固定値(マニュアル指定値)
    SwitchProtocol_TypeA_data_object.Tag    = 0x8F;
    SwitchProtocol_TypeA_data_object.Length = 0x02;
    SwitchProtocol_TypeA_data_object.Value[0] = 0x00; //StandardType = A
    SwitchProtocol_TypeA_data_object.Value[1] = 0x04; //Layer = 4

    _assemblyAPDUcommand_SwitchProtocol_Base(SwitchProtocol_TypeA_data_object);

    return;
}

void assemblyAPDUcommand_SwitchProtocol_TypeB_AutoActivate(void){

    APDU_DATA_OBJECT SwitchProtocol_TypeB_data_object = {0};

    //固定値(マニュアル指定値)
    SwitchProtocol_TypeB_data_object.Tag    = 0x8F;
    SwitchProtocol_TypeB_data_object.Length = 0x02;
    SwitchProtocol_TypeB_data_object.Value[0] = 0x01; //StandardType = B;
    SwitchProtocol_TypeB_data_object.Value[1] = 0x04; //Layer = 4

    _assemblyAPDUcommand_SwitchProtocol_Base(SwitchProtocol_TypeB_data_object);

    return;
}

//4.11章 Reset Device
//このファイルはAPDU階層の話なのでResetDeviceのACK送信はアプリ層側で処理する
void assemblyAPDUcommand_ResetDevice(void){

    APDU_COMMAND resetDevice_apdu_command = {0};

    //固定値(マニュアル指定値)
    resetDevice_apdu_command.CLA = 0xFF;
    resetDevice_apdu_command.INS = 0xC2;
    resetDevice_apdu_command.P1  = 0x00;
    resetDevice_apdu_command.P2  = 0x00;
    resetDevice_apdu_command.Lc  = LC_NO_DATA;     //Lcなし
    //DataInなし
    //Leなし

    passToCcidLayer(resetDevice_apdu_command);

    return;
}

//4.12章 Get Firmware Version
void assemblyAPDUcommand_GetFirmwareVersion(void){
    
    APDU_COMMAND getFirmwareVersion_apdu_command = {0};

    //固定値(マニュアル指定値)
    getFirmwareVersion_apdu_command.CLA = 0xFF;
    getFirmwareVersion_apdu_command.INS = 0x56;
    getFirmwareVersion_apdu_command.P1  = 0x00;
    getFirmwareVersion_apdu_command.P2  = 0x00;
    getFirmwareVersion_apdu_command.Lc  = LC_NO_DATA;     //Lcなし
    //DataInなし
    //Leなし

    passToCcidLayer(getFirmwareVersion_apdu_command);

    return;
}

/********/
/* BASE */
/********/

//DataObject を受け取って ManageSession APDU コマンドを組み立て
void _assemblyAPDUcommand_ManageSession_Base(const APDU_DATA_OBJECT input_data_object){

    APDU_COMMAND output_apdu_command = {0};
   
    //固定値ヘッダー(マニュアル指定値)
    output_apdu_command.CLA = 0xFF;
    output_apdu_command.INS = 0xC2;
    output_apdu_command.P1  = 0x00;
    output_apdu_command.P2  = 0x00;

    //Lcはデータ長で変わるため最後に組み付ける

    //Leの組付け
    output_apdu_command.Le = LE_NO_USE;

    //入力データのタグサイズを確認する
    uint8_t input_tag_size = _checkTagSize(input_data_object.Tag);

    //Lc設定
    //入力データの長さを計算
    output_apdu_command.Lc = input_tag_size + DATA_OBJECT_LENGTH_SIZE + input_data_object.Length; //Tag + Length + Value

    //DataIn設定
    //Tag
    _assemblyTag(&output_apdu_command, input_data_object, COMMAND_DATA_IN_NO_OFFSET); //DataIn部の先頭から開始
    //Length
    output_apdu_command.DataIn[input_tag_size] = input_data_object.Length;
    //Value
    _assemblyValue(&output_apdu_command, input_data_object, input_tag_size + DATA_OBJECT_LENGTH_SIZE); //Tag,Length分のオフセット

    //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ
    passToCcidLayer(output_apdu_command);

    return;
}


//DataObject を受け取って Transparent Exchange APDU コマンドを組み立て
void _assemblyAPDUcommand_TransparentExchange_Base(APDU_DATA_OBJECT input_data_object,const uint16_t timeout_ms){

    APDU_COMMAND output_apdu_command = {0};
   
    //固定値ヘッダーの組付け(マニュアル指定値)
    output_apdu_command.CLA = 0xFF;
    output_apdu_command.INS = 0xC2;
    output_apdu_command.P1  = 0x00;
    output_apdu_command.P2  = 0x01;

    //Lcはデータ長で変わるため最後に組み付ける

    //Leの組付け
    output_apdu_command.Le = LE_NO_USE;

    //処理用変数
    APDU_DATA_OBJECT timer_data_object = {0};
    APDU_DATA_OBJECT data_object = {0};
    uint8_t Lc_timer = 0;
    uint8_t Lc_data  = 0;

    //DataInの組付け

    //タイマーが設定されていたらタイマーをDataInに組み付ける
    if(timeout_ms > 0){
        timer_data_object = _getTimerDataObject(timeout_ms);
        Lc_timer = DATA_OBJECT_TAG_SIZE + DATA_OBJECT_LENGTH_SIZE + timer_data_object.Length; //Tag + Length + Value
        output_apdu_command.DataIn[0] = (uint8_t)((0xFF00 & timer_data_object.Tag) >> 8);
        output_apdu_command.DataIn[1] = (uint8_t)(0x00FF & timer_data_object.Tag);
        output_apdu_command.DataIn[2] = timer_data_object.Length;
        for(uint8_t i = 0; i < timer_data_object.Length; i++){
            output_apdu_command.DataIn[i + 3] = timer_data_object.Value[i];
        }
    }

    //入力データオブジェクトのタグサイズを確認する
    uint8_t input_tag_size = _checkTagSize(input_data_object.Tag);

    //入力データオブジェクトの長さを計算
    Lc_data = input_tag_size + DATA_OBJECT_LENGTH_SIZE + input_data_object.Length; //Tag + Length + Value

    //入力データオブジェクトをDataInに組み付ける
    //タイマーがある場合はタイマー分オフセットするのでLc_Timerを足すこと
    //Tag
    _assemblyTag(&output_apdu_command, input_data_object, Lc_timer);    //タイマーデータオブジェクト分のオフセット
    //Length
    output_apdu_command.DataIn[Lc_timer + input_tag_size] = input_data_object.Length;
    //Value
    _assemblyValue(&output_apdu_command, input_data_object, Lc_timer + input_tag_size + DATA_OBJECT_LENGTH_SIZE); //Tag,Length分のオフセット

    //Lcの組付け
    output_apdu_command.Lc = (uint8_t)(Lc_data + Lc_timer);

    //組み立てたAPDUコマンド構造体を下位のCCIDコマンドレイヤにパス
    passToCcidLayer(output_apdu_command);

    return;
}


//DataObject を受け取って SwitchProtocol APDU コマンドを組み立て
void _assemblyAPDUcommand_SwitchProtocol_Base(const APDU_DATA_OBJECT input_data_object){

    APDU_COMMAND output_apdu_command = {0};
   
    //固定値ヘッダー(マニュアル指定値)
    output_apdu_command.CLA = 0xFF;
    output_apdu_command.INS = 0xC2;
    output_apdu_command.P1  = 0x00;
    output_apdu_command.P2  = 0x02;

    //Lcはデータ長で変わるため最後に組み付ける

    //Leの組付け
    output_apdu_command.Le = LE_NO_USE;

    //入力データのタグサイズを確認する
    uint8_t input_tag_size = _checkTagSize(input_data_object.Tag);

    //Lc設定
    //入力データの長さを計算
    output_apdu_command.Lc = input_tag_size + DATA_OBJECT_LENGTH_SIZE + input_data_object.Length; //Tag + Length + Value

    //DataIn設定
    //Tag
    _assemblyTag(&output_apdu_command, input_data_object, COMMAND_DATA_IN_NO_OFFSET); //DataIn部の先頭から開始
    //Length
    output_apdu_command.DataIn[input_tag_size] = input_data_object.Length;
    //Value
    _assemblyValue(&output_apdu_command, input_data_object, input_tag_size + DATA_OBJECT_LENGTH_SIZE); //Tag,Length分のオフセット

    //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ
    passToCcidLayer(output_apdu_command);

    return;
}

/**********/
/* 道具箱 */
/**********/

uint8_t _checkTagSize(const uint16_t input_data_object_tag){
    //入力データのタグサイズを確認する
    uint8_t input_tag_size = 0;

    if(input_data_object_tag > 0xFF){
        input_tag_size = 2;}
    else{   
        input_tag_size = 1;
    }

    return input_tag_size;
}

//指定されたAPDUコマンドのDataIn部に、指定されたオフセット位置から指定されたDataObjectのTagを格納する
void _assemblyTag(APDU_COMMAND* processing_target_apdu_command, const APDU_DATA_OBJECT input_data_object, const uint8_t offset){
    //入力データのタグサイズを確認する
    uint8_t input_tag_size = _checkTagSize(input_data_object.Tag);

    if(input_tag_size == 1){
        processing_target_apdu_command->DataIn[offset] = (uint8_t)(input_data_object.Tag & 0x00FF);
    }else if(input_tag_size == 2){   
        processing_target_apdu_command->DataIn[offset]     = (uint8_t)((0xFF00 & input_data_object.Tag) >> 8);
        processing_target_apdu_command->DataIn[offset + 1] = (uint8_t)(0x00FF & input_data_object.Tag);
    }else{
        //error(); //タグサイズがおかしい
    }
    
    return;
}

//指定されたAPDUコマンドのDataIn部に、指定されたオフセット位置から指定されたDataObjectのValueを格納する
void _assemblyValue(APDU_COMMAND* processing_target_apdu_command, const APDU_DATA_OBJECT input_data_object, const uint8_t offset){
    if(input_data_object.Length > 0){
        //ValueがあるDataObject
        for(uint8_t i = 0; i < input_data_object.Length; i++){
            processing_target_apdu_command->DataIn[offset + i] = input_data_object.Value[i];
        }
    }else{
        //Valueが無いDataObject
        //何もしない
    }
    return;
}

//ms単位でタイムアウト時間を受け取り、APDU_DATA_OBJECTに変換(DataObjectではus指定だがRC-S660/Sでは精度がms単位)
APDU_DATA_OBJECT _getTimerDataObject(const uint16_t timeout_ms){
    APDU_DATA_OBJECT timer_data_object = {0};

    //固定値(マニュアル指定値)
    timer_data_object.Tag    = 0x5F46;
    timer_data_object.Length = 0x04;

    //タイマー::変数
    uint32_t timeout_us_littleEndian = 0x00000000;

    //タイムアウトmsをusに変換
    uint32_t timeout_us_BigEndian    = (uint32_t)(timeout_ms * 1000);

    //タイムアウトをリトルエンディアンに変換
    timeout_us_littleEndian = bigEndianToLittleEndian(timeout_us_BigEndian);

    //タイムアウト時間を格納
    timer_data_object.Value[0] = (uint8_t)((0xFF000000 & timeout_us_littleEndian) >> 24);
    timer_data_object.Value[1] = (uint8_t)((0x00FF0000 & timeout_us_littleEndian) >> 16);
    timer_data_object.Value[2] = (uint8_t)((0x0000FF00 & timeout_us_littleEndian) >> 8);
    timer_data_object.Value[3] = (uint8_t)(0x000000FF & timeout_us_littleEndian);

    return timer_data_object;
}