#ifndef IOT_FAN_H
#define IOT_FAN_H

extern u32 g_u32FanFbSpeed;
extern s8 SetFanSpeed(u8 fanspeed);
void FanInit();
void ProcessSht21Data();
u8 GetFanSpeed();
#define FAN_DEFAULT_PWM 55
#endif         