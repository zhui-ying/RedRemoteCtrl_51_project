#ifndef __AT24C02_H__
#define __AT24C02_H__

#include <reg52.h>
#include <intrins.h>

#define FOSC 11059200L //晶振设置，默认使用11.0592M Hz
//#define FOSC 12000000L //晶振设置，使用12M Hz
//#define FOSC 24000000L //晶振设置，使用24M Hz

//AT24C02接口
sbit X24_sda=P2^0;			 //IO口定义
sbit X24_scl=P2^1;

void X24c02Init();
void X24c02WriteAdd(unsigned char address,unsigned char info);
unsigned char X24c02ReadAdd(unsigned char address);
void X24c16WriteAdd(unsigned int address,unsigned char info);
unsigned char X24c16ReadAdd(unsigned int address);
#endif
