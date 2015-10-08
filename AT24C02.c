#include "AT24C02.h"

/*******************************************************************************
* 函 数 名 ：Delayus
* 函数功能 ：实现 us级的延时
* 输    入 ：us
* 输    出 ：无
*******************************************************************************/
void Delayus(unsigned int us)
{
	unsigned int i;
	for(i=0;i<us;i++);
}

/////////24C08读写驱动程序////////////////////
/*******************************************************************************
* 函 数 名 ：X24c02Init
* 函数功能 ：24C02初始化子程序
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void X24c02Init()
{
	X24_scl=1; _nop_(); X24_sda=1; _nop_();
}

/*******************************************************************************
* 函 数 名 ：X24c02Start
* 函数功能 ：启动24c02 I2C总线
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void X24c02Start()		
{
	X24_sda=1; _nop_(); X24_scl=1; _nop_(); X24_sda=0; _nop_(); X24_scl=0; _nop_();
}

/*******************************************************************************
* 函 数 名 ：X24c02Stop
* 函数功能 ：停止I2C总线
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void X24c02Stop() 		
{
	X24_sda=0; _nop_(); X24_scl=1; _nop_(); X24_sda=1; _nop_();
}

/*******************************************************************************
* 函 数 名 ：X24c02WriteByte
* 函数功能 ：写一个字节
* 输    入 ：j 需要写入的值
* 输    出 ：无
*******************************************************************************/
void X24c02WriteByte(unsigned char j)  
{
	unsigned char i,temp;
	temp=j;
	for (i=0;i<8;i++)
	{temp=temp<<1; X24_scl=0; _nop_(); X24_sda=CY; _nop_(); X24_scl=1; _nop_();}
	X24_scl=0; _nop_(); X24_sda=1; _nop_();
}

/*******************************************************************************
* 函 数 名 ：X24c02ReadByte
* 函数功能 ：读一个字节
* 输    入 ：无
* 输    出 ：读取的值
*******************************************************************************/
unsigned char X24c02ReadByte()	
{
	unsigned char i,j,k=0;
	X24_scl=0; _nop_(); X24_sda=1;
	for (i=0;i<8;i++)
	{
		_nop_(); X24_scl=1; _nop_();
		if (X24_sda==1) j=1;
		else j=0;
		k=(k<<1)|j;
		X24_scl=0;
	}
	_nop_(); return(k);
}

/*******************************************************************************
* 函 数 名 ：X24c02Clock
* 函数功能 ：I2C总线时钟
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void X24c02Clock()		 
{
	unsigned char i=0;
	X24_scl=1; _nop_();
	while ((X24_sda==1)&&(i<255))i++;
	X24_scl=0; _nop_();
}

/*******************************************************************************
* 函 数 名 ：X24c02ReadAdd
* 函数功能 ：从24c02的地址address中读取一个字节数据
* 输    入 ：address
* 输    出 ：在该地址下读取的值
*******************************************************************************/
unsigned char X24c02ReadAdd(unsigned char address)
{
	unsigned char i;
	X24c02Start(); X24c02WriteByte(0xa0);
	X24c02Clock(); X24c02WriteByte(address);
	X24c02Clock(); X24c02Start();
	X24c02WriteByte(0xa1); X24c02Clock();
	i=X24c02ReadByte(); X24c02Stop();
	Delayus(10);
	return(i);
}

/*******************************************************************************
* 函 数 名 ：X24c02ReadAdd
* 函数功能 ：从24c02的地址address中读取一个字节数据
* 输    入 ：address
* 输    出 ：在该地址下读取的值
*******************************************************************************/
unsigned char X24c16ReadAdd(unsigned int address)
{
	unsigned char i,temp_h,temp_l;
	temp_h = (unsigned char)((address & 0x700)>>7);
	temp_l = (unsigned char)(address & 0x00ff);
	X24c02Start(); X24c02WriteByte(0xa0 | temp_h);
	X24c02Clock(); X24c02WriteByte(temp_l);
	X24c02Clock(); X24c02Start();
	X24c02WriteByte(0xa1 | temp_h); X24c02Clock();
	i=X24c02ReadByte(); X24c02Stop();
	Delayus(10);
	return(i);
}

/*******************************************************************************
* 函 数 名 ：X24c02WriteAdd
* 函数功能 ：向24c02的address地址中写入一字节数据
* 输    入 ：address 地址 info 值
* 输    出 ：无
*******************************************************************************/
void X24c02WriteAdd(unsigned char address,unsigned char info)
{
	EA=0;
	X24c02Start(); X24c02WriteByte(0xa0);
	X24c02Clock(); X24c02WriteByte(address);
	X24c02Clock(); X24c02WriteByte(info);
	X24c02Clock(); X24c02Stop();
	EA=1;
	Delayus(50);
}

/*******************************************************************************
* 函 数 名 ：X24c02WriteAdd
* 函数功能 ：向24c02的address地址中写入一字节数据
* 输    入 ：address 地址 info 值
* 输    出 ：无
*******************************************************************************/
void X24c16WriteAdd(unsigned int address,unsigned char info)
{
		unsigned char temp_h,temp_l;
	temp_h = (unsigned char)((address & 0x700)>>7);
	temp_l = (unsigned char)(address & 0x00ff);
	
	EA=0;
	X24c02Start(); X24c02WriteByte(0xa0 | temp_h);
	X24c02Clock(); X24c02WriteByte(temp_l);
	X24c02Clock(); X24c02WriteByte(info);
	X24c02Clock(); X24c02Stop();
	EA=1;
	Delayus(50);
}
