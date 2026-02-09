// myUtil.h
// Utility functions: LCD helpers, EEPROM, SPI, preset management

#pragma once

// String utilities
extern void CopyCharByLength(char *org, char *dest, int max_length);

// RTC (real-time clock via NTP)
extern void getRTC(char* timestr);
extern void LcdPrintRTC(void);

// LCD display helpers
extern void EraseLCD(int posx, int posy, int numOfChar);
extern void PrintInt2Digit(int value, int px, int py);
extern void PrintInt3Digit(int value, int px, int py);
extern void PrintInt4Digit(int value, int px, int py);

// EEPROM persistence
extern int load_eeprom();
extern void save_eeprom();

// SPI / digital potentiometer (AD5263)
extern void SetupAD5263(void);
extern void MySpiTransfer(int SEG7, int HC595, int AD5263_ADDR, int AD5263_DATA);

// Preset management
extern void LoadPresetToCurrentParam(int prst_no);
extern void CheckAndCorrectPreset(void);

// Integer clamp utilities
extern bool UpdateClampedInt(int *target, int value, int minValue, int maxValue);
extern bool StepClampedInt(int *target, int delta, int minValue, int maxValue);

// (unused in normal operation)
extern void TempoLedHandler(void);
extern int GetTempoTargetDelayTime(int appTime);
