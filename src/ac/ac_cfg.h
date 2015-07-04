/**
******************************************************************************
* @file     ac_hal.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    
******************************************************************************
*/

#ifndef  __AC_CFG_H__ 
#define  __AC_CFG_H__




#define DEVICE_ID "9999999999999922" //必须填写8个字节，不足的前面补0

#define MAJOR_DOMAIN_ID 18//主域id

#define SUB_DOMAIN_ID  19 //子域id


#define DEFAULT_IOT_CLOUD_KEY {\
    0xb8, 0xd9, 0x35, 0xe4,\
    0xd6, 0xd8, 0xf2, 0xd6,\
    0xc8, 0x28, 0x2f, 0x9f,\
    0xd9, 0x62, 0x48, 0xc7,\
    0x96, 0xa9, 0xed, 0x5b,\
    0x5a, 0x1a, 0x95, 0x59,\
    0xd2, 0x3c, 0xbb, 0x5f,\
    0x1b, 0x03, 0x07, 0x3f,\
    0x01, 0x00, 0x01\
}

#define DEFAULT_IOT_PRIVATE_KEY {\
    0x8d, 0xc2, 0xf2, 0xae,\
    0xbc, 0x7b, 0xdc, 0x93,\
    0x80, 0xde, 0xb9, 0x6c,\
    0xa0, 0xe3, 0x93, 0x5d,\
    0x89, 0x30, 0x7f, 0x1e,\
    0xec, 0x7f, 0xd3, 0x75,\
    0x3c, 0x3d, 0x4b, 0xd6,\
    0x59, 0x8a, 0xb3, 0x2b,\
    0xea, 0x76, 0xb5, 0x4d,\
    0x3a, 0xb8, 0x75, 0xfc,\
    0xbb, 0xc6, 0x49, 0xa3,\
    0x48, 0x5b, 0x89, 0x2f,\
    0x9a, 0xc8, 0x65, 0x68,\
    0x9a, 0x07, 0x15, 0x71,\
    0x03, 0x92, 0x10, 0x0e,\
    0x8c, 0x7b, 0x7e, 0xc5,\
    0x1e, 0x8b, 0x9c, 0x8f,\
    0x15, 0x29, 0x56, 0xe5,\
    0xb8, 0x28, 0xdb, 0x0b,\
    0x14, 0xb0, 0x28, 0x2d,\
    0x7d, 0x68, 0xd9, 0x21,\
    0x83, 0x8b, 0x31, 0xc8,\
    0x3b, 0xc8, 0x60, 0xd2,\
    0x28, 0xe8, 0xf6, 0xb1,\
    0xcb, 0x26, 0x19, 0x68,\
    0x7c, 0x69, 0x4a, 0x80,\
    0xa5, 0x80, 0xd8, 0x0c,\
    0x3f, 0x43, 0x8f, 0x95\
}

#endif
/******************************* FILE END ***********************************/