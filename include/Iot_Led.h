#ifndef IOT_LED_H
#define IOT_LED_H

#define  OFF 0
#define  ON 1
#define  QUERY 2
extern s8 SetLedOnOff(u8 ledcontrol);
extern s8 GetLedOnOff();
#endif         