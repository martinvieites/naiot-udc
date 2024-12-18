#include <Matrix.h>

uint8_t LedArray10[8] = { 0xE6, 0x49, 0x49, 0x49, 0x49, 0x49, 0xC9, 0x46 };
uint8_t LedArray9[8] = { 0x3C, 0x42, 0x02, 0x3E, 0x42, 0x42, 0x42, 0x3C };
uint8_t LedArray8[8] = { 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C };
uint8_t LedArray7[8] = { 0x20, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0x7E };
uint8_t LedArray6[8] = { 0x3C, 0x42, 0x42, 0x42, 0x7C, 0x40, 0x42, 0x3C };
uint8_t LedArray5[8] = { 0x3C, 0x42, 0x02, 0x02, 0x7C, 0x40, 0x40, 0x7E };
uint8_t LedArray4[8] = { 0x02, 0x02, 0x7E, 0x42, 0x22, 0x12, 0x0e, 0x06 };
uint8_t LedArray3[8] = { 0x3C, 0x42, 0x02, 0x02, 0x1C, 0x02, 0x42, 0x3C };
uint8_t LedArray2[8] = { 0x7E, 0x20, 0x10, 0x08, 0x04, 0x02, 0x42, 0x3C };
uint8_t LedArray1[8] = { 0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x18, 0x08 };
uint8_t LedArray0[8] = { 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C };
uint8_t LedArrayStop[8] = { 0x3C, 0x42, 0x81, 0xBD, 0xBD, 0x81, 0x42, 0x3C };

uint8_t LEDArray[8];

uint8_t* estados[12] = { LedArray10, LedArray9, LedArray8, LedArray7, LedArray6, LedArray5, LedArray4, LedArray3, LedArray2, LedArray1, LedArray0, LedArrayStop };