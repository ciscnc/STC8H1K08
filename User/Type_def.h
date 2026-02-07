/**
 * @file type_def.h
 * @brief 类型定义头文件，定义通用数据类型和宏
 *
 * @date 2026-02-06
 */
#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

//========================================================================
//                               类型定义
//========================================================================

typedef unsigned char u8;   //  8 bits
typedef unsigned int u16;   // 16 bits
typedef unsigned long u32;  // 32 bits

typedef signed char int8_t;   //  8 bits
typedef signed int int16_t;   // 16 bits
typedef signed long int32_t;  // 32 bits

typedef unsigned char uint8_t;   //  8 bits
typedef unsigned int uint16_t;   // 16 bits
typedef unsigned long uint32_t;  // 32 bits

//========================================================================
//                               宏定义
//========================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef bool
#define bool uint8_t
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif


#endif /* __TYPE_DEF_H__ */
