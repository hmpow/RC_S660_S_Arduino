#include "rcs660s_ccid.h"


//APDUコマンドから PC_to_RDR_Escape CCID コマンドを組み立て
//コマンド = 0x68 <LEN lttleEndian 4byte> 0x00 <SeqNo> <RFU 00 00 00> [DATA PACKET]
void assemblyCCIDcommand_PC_to_RDR_Escape(const uint8_t APDU_data[], const uint32_t APDU_lenBigendian, const uint8_t SeqNo){

    //定数(マニュアル指定値)
    const uint8_t bMessageType = 0x6B;
    const uint8_t bSlot        = 0x00;
    const uint8_t RFUbyte      = 0x00;
    const uint8_t START_OFFSET = 10;

    //変数たち
    uint32_t dwLength = 0x00000000; //データの長さ
    uint8_t*  arrCCIDcommand = NULL;       //出力CCIDコマンドフレーム
    
    //長すぎたらエラー
    if(APDU_lenBigendian > 0xFFFF - START_OFFSET){
        debugPrintMsg("ERROR! assemblyCCIDcommand_PC_to_RDR_Escape データが長すぎます\n");
        exit(1);
    }


    //card_command＋リーダライタコマンド分のメモリを確保
    size_t arrSize = (size_t)(START_OFFSET + APDU_lenBigendian);
    arrCCIDcommand = (uint8_t*)malloc(sizeof(uint8_t) * arrSize);

	if (arrCCIDcommand == NULL){
		debugPrintMsg("malloc ERROR!\n");
		exit(1);
	}

    //APDUコマンドの前にCCID_PC_to_RDR_Escapeコマンド結合
    arrCCIDcommand[0] = bMessageType;
    
    //dwLength リトルエンディアンにして詰め込み
    dwLength = bigEndianToLittleEndian(APDU_lenBigendian);
    
    arrCCIDcommand[1] = (uint8_t)((0xFF000000 & dwLength) >> 24);
    arrCCIDcommand[2] = (uint8_t)((0x00FF0000 & dwLength) >> 16);
    arrCCIDcommand[3] = (uint8_t)((0x0000FF00 & dwLength) >> 8);
    arrCCIDcommand[4] = (uint8_t)(0x000000FF & dwLength);
    
    arrCCIDcommand[5] = bSlot;
    arrCCIDcommand[6] = SeqNo;
    arrCCIDcommand[7] = RFUbyte;
    arrCCIDcommand[8] = RFUbyte;
    arrCCIDcommand[9] = RFUbyte;
    
    //パケットデータ配列に流し込み
    uint32_t i = 0;
    for(i = 0; i < APDU_lenBigendian; i++){
        arrCCIDcommand[i + START_OFFSET] = APDU_data[i]; //DATA
    }

    //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ

    assemblyRcs660sUartCommandFrame(arrCCIDcommand, (uint16_t)((0xFFFF & APDU_lenBigendian) + START_OFFSET));

    free(arrCCIDcommand);
    
    return;
}


//指定シーケンスNoの PC_to_RDR_Abort コマンドを組み立て
//コマンド = 0x72 <LEN 00 00 00 00> 0x00 <SeqNo> <RFU 00 00 00>
void assemblyCCIDcommand_PC_to_RDR_Abort(uint8_t SeqNo){
    //定数(マニュアル指定値)
    const uint8_t  bMessageType = 0x72;
    const uint32_t dwLength     = 0x00000000;
    const uint8_t  bSlot        = 0x00;
    const uint8_t  abRFU_byte   = 0x00;

    //変数
    uint8_t bSeq = SeqNo;
    uint8_t arrCCIDcommand[1 + 4 + 1 + 1 + 3];

    //CCID_PC_to_RDR_Abortコマンド結合
    arrCCIDcommand[0] = bMessageType;
    arrCCIDcommand[1] = (uint8_t)((0xFF000000 & dwLength) >> 24);
    arrCCIDcommand[2] = (uint8_t)((0x00FF0000 & dwLength) >> 16);
    arrCCIDcommand[3] = (uint8_t)((0x0000FF00 & dwLength) >> 8);
    arrCCIDcommand[4] = (uint8_t)(0x000000FF & dwLength);
    arrCCIDcommand[5] = bSlot;
    arrCCIDcommand[6] = bSeq;
    arrCCIDcommand[7] = abRFU_byte;
    arrCCIDcommand[8] = abRFU_byte;
    arrCCIDcommand[9] = abRFU_byte;

     //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ
    assemblyRcs660sUartCommandFrame(arrCCIDcommand, (uint16_t)(1 + 4 + 1 + 1 + 3));

    return;
}