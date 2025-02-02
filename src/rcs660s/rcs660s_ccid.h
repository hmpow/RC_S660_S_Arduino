#ifndef RCS660S_CCID_H
#define RCS660S_CCID_H

#include <stdlib.h>
//RC-S660/Sでサポート・カスタムされたCCIDコマンドを組み立てる
#include <stdint.h>

#include "rcs660s_util.h"
#include "rcs660s_uart.h"

void assemblyCCIDcommand_PC_to_RDR_Escape(const uint8_t*, const uint32_t, const uint8_t);
void assemblyCCIDcommand_PC_to_RDR_Abort(uint8_t SeqNo);

#endif // RCS660S_CCID_H