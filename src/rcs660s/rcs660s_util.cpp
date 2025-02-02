#include "rcs660s_util.h"

uint32_t bigEndianToLittleEndian(const uint32_t bigEendianInput){
    uint32_t littleEndianOutput = 0x00000000;

    littleEndianOutput = (bigEendianInput & 0xFF000000) >> 24
                         | ((bigEendianInput & 0x00FF0000) >> 8)
                         | ((bigEendianInput & 0x0000FF00) << 8)
                         | ((bigEendianInput & 0x000000FF) << 24);

    return littleEndianOutput;
}