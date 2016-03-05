#ifndef CRC_H
#define CRC_H
#include <stdint.h>
#include <stdio.h>
uint16_t gen_crc(const unsigned char *byteArray, int length)
{
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *byteArray++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}   

int test_crc(const unsigned char *byteArray, int length, unsigned short crc_test){
    unsigned char x;
    unsigned short crc = 0xFFFF;
    while (length--){
        x = crc >> 8 ^ *byteArray++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    if(crc == crc_test){
        return 1;
    }
    return 0;
}
#endif