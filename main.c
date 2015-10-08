/********************************* 深圳市航太电子有限公司 *******************************
* 实 验 名 ：红外键值记录和发射
* 实验说明 ：记录任意以38K载波发送的红外信号存储到外部存储芯片，并映射到矩阵键盘，按下按键时即发射之前记录的红外信号
* 实验平台 ：航太51单片机开发板 V1.1
* 连接方式 ：跳线帽CN4 引脚2,4相接 跳线帽CN20 引脚2,4相接，跳线帽CN3 引脚1,3相接
* 注    意 ：程序通过关断和打开TR1来实现发送低电平和高电平，打开TR1时，发送38k高电平载波，接收端经过反相后接收到低电平，
*            关断TR1时，发送端不发送载波，接收端经反相后接收到高电平
* 作    者 ：航太电子产品研发部    QQ ：1909197536
* 店    铺 ：http://shop120013844.taobao.com/
****************************************************************************************/
#include <reg52.h> 
#include "AT24C02.h"
#include <stdio.h>
#include <string.h>

#define FOSC 11059200L //晶振设置，默认使用11.0592M Hz
#define KEY_BLOCK (sizeof(struct KEYMSG)) //存储一个键值的信息需要的空间大小
//IO接口定义
sbit IR_OUT=P1^6 ;
//sbit dula=P2^6;
//sbit wela=P2^7;
sbit IRIN = P3^2;         //红外接收器数据线
sbit BEEP = P1^5;         //蜂鸣器驱动线
sbit LED_read = P1^0;
sbit LED_ok = P1^1;
sbit LED_run = P1^2;

#define KEY_PORT P3
//系统状态定义
#define SEND_RED 0  //默认的发送状态
#define READ_RED 1  //读取红外信号状态
#define READ_OK  2  // 读取成功，等待按键匹配状态

struct KEYMSG{
	unsigned char key_value;//对应到矩阵键盘键值
	unsigned int count_bit;//该红外信号高低电平总个数，一般是bit*2
	unsigned char value_msg[40];//记录高低电平
	unsigned char  time[640];//记录每一个电平持续时间,每两个一组，分别存放TH0，TL0
};

volatile struct KEYMSG key_info;
data volatile unsigned char sys_status = SEND_RED;//系统当前状态

unsigned char KeyScan();
void beep();
void ReadKeyMsg(struct KEYMSG *key_msg);
void WriteKeyMsg(struct KEYMSG *key_msg);
void SendKeyMsg(struct KEYMSG *key_msg);
void PrintDebugMsg();

/*******************************************************************************
* 函 数 名 ：Delayms
* 函数功能 ：实现 ms级的延时
* 输    入 ：ms
* 输    出 ：无
*******************************************************************************/
void Delayms(unsigned int ms)
{
	unsigned int i,j;
	for(i=0;i<ms;i++)
	#if FOSC == 11059200L
		for(j=0;j<114;j++);
	#elif FOSC == 12000000L
	  for(j=0;j<123;j++);
	#elif FOSC == 24000000L
		for(j=0;j<249;j++);
	#else
		for(j=0;j<114;j++);
	#endif
}

/*******************************************************************************
* 函 数 名 ：Init_Timer
* 函数功能 ：定时器1初始化 13us产生一次中断 用于产生38K载波 
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Init_Timer(void) 
{ 
	TMOD=0x21;	 //T0 mode 1      T1 mode 2  	
	TH1=256-(1000*11.02/38.0/12)/2+0.5;						
	//特殊
	TL1=TH1;
	ET1=1; 
	TR1=0;
//	EA=1; 
} 

/*******************************************************************************
* 函 数 名 ：Exit0Init
* 函数功能 ：外中断0初始化程序
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Exit0Init()
{
	EX0 = 0;	//禁能 INT1 外部中断

    IT0 = 1;	// 触发方式为脉冲负边沿触发
 //   EA = 1;//总中断
}   

void UartInit()
{
	SCON |= 0x50;
	TL2 = RCAP2L = (65536-(FOSC/32/9600));
	TH2 = RCAP2H = (65536-(FOSC/32/9600))>>8;
	T2CON = 0x34;
	//ES = 1;
}

void PutSring(char *buf)
{
	int i;
	for(i=0;i<strlen(buf);i++)
	{
		SBUF = buf[i];//写入SBUF，开始发送，后面就自动进入中断发送
		while(!TI);		  //等待发送数据完成
		TI=0;			  //清除发送完成标志位
	}
}

/*******************************************************************************
* 函 数 名 ：putchar
* 函数功能 ：代替系统自带的putchar函数，实现printf功能
* 输    入 ：要发送的参数
* 输    出 ：发送成功的参数
*******************************************************************************/
char putchar(char ch)
{ 
	/* Place your implementation of fputc here */ 
	SBUF=(unsigned char)ch; //将接收到的数据放入到发送寄存器
	while(!TI);		  //等待发送数据完成
	TI=0;		 //清除发送完成标志位	
	return ch;
}

/*******************************************************************************
* 函 数 名 ：main
* 函数功能 ：主函数
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void main(void) 
{ 
  unsigned char date = 0;
//	xdata unsigned int aa;
	IR_OUT=1;
	UartInit();
	Init_Timer(); 
	X24c02Init();
	Exit0Init();
	LED_read = 1;
	LED_ok = 1;
	EA = 1;//打开总中断
//	printf("system init OK\r\n");
	
//			TH0 = 0;
//		TL0 = 0;
//		TR0 = 1;
//	Delayms(20);
//	TR0 = 0;
//	printf("test TH0 %d\r\n",(int)TH0);
//	printf("test TL0 %d\r\n",(int)TL0);
	//aa = TL0;
	//aa = TH0*256;
//	aa = (unsigned int)(TH0*256 + TL0);
//	printf("test value %u\r\n",aa);
//	aa = aa;



//key_info.key_value = 1;
//key_info.count_bit = 67;
//key_info.value_msg[0] = 85;key_info.value_msg[1] = 85;key_info.value_msg[2] = 85;key_info.value_msg[3] = 85;
//key_info.time[0] = 8315;key_info.time[1] = 8315;key_info.time[2] = 8315;key_info.time[3] = 8315;key_info.time[4] = 8315;key_info.time[5] = 8315;
//SendKeyMsg(&key_info);

	while(1) 
	{	
		date = 0xff;
		LED_run = !LED_run;
		Delayms(20);
		if(sys_status != READ_RED)
		{
			date = KeyScan();
		}
		if(date != 0xff)
		{
			if(date == 16)//进入读模式
			{
				memset(&key_info.key_value , 0 , sizeof(struct KEYMSG));//清空buf
				sys_status = READ_RED;//进入红外读取状态
				LED_read = 0;				
				IE0 = 0;
				EX0 = 1;//打开外部中断
			}
			
			if(sys_status == SEND_RED)//从存储器读取参数，并通过红外发送发送键值
			{
				if(date <=10)//不超过10个值
				{
					key_info.key_value = date;
					ReadKeyMsg(&key_info);
					//PrintDebugMsg();
					SendKeyMsg(&key_info);
					beep();
				} 
			}
			
			if(sys_status == READ_OK)//匹配接收到的键值
			{
				if(date <=10) //不超过10个值
				{
					key_info.key_value = date;
					PrintDebugMsg();
					WriteKeyMsg(&key_info);					
					LED_read = 1;
					LED_ok = 1;//灭灯
					sys_status = SEND_RED;
				} 
			}
		}
	} 
} 

/*******************************************************************************
* 函 数 名 ：KeyScan
* 函数功能 ：4*4键盘扫描
* 输    入 ：无
* 输    出 ：num 获取的键值，若没有键值则返回 0xff
*******************************************************************************/
unsigned char KeyScan()
{
	unsigned char temp,num;
	num = 0xff;
	KEY_PORT=0xfe;
	temp=KEY_PORT;
	temp=temp&0xf0;
	while(temp!=0xf0)
	{
		Delayms(5); //延时消抖
		temp=KEY_PORT;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			temp=KEY_PORT;
		switch(temp)
			{
				case 0xee:num=1;
					break;
				case 0xde:num=2;
					break;
				case 0xbe:num=3;
					break;
				case 0x7e:num=4;
					break;
			}
		while(temp!=0xf0)
			{
				temp=KEY_PORT;
				temp=temp&0xf0;
			}
		}
		Delayms(5); //延时消抖
	}

	KEY_PORT=0xfd;
	temp=KEY_PORT;
	temp=temp&0xf0;
	while(temp!=0xf0)
	{
		Delayms(5); //延时消抖
		temp=KEY_PORT;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			temp=KEY_PORT;
		switch(temp)
			{
				case 0xed:num=5;
					break;
				case 0xdd:num=6;
					break;
				case 0xbd:num=7;
					break;
				case 0x7d:num=8;
					break;
			}
		while(temp!=0xf0)
			{
				temp=KEY_PORT;
				temp=temp&0xf0;
			}
		}
		Delayms(5); //延时消抖
	}

	KEY_PORT=0xfb;
	temp=KEY_PORT;
	temp=temp&0xf0;
	while(temp!=0xf0)
	{
		Delayms(5); //延时消抖
		temp=KEY_PORT;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			temp=KEY_PORT;
		switch(temp)
			{
				case 0xeb:num=9;
					break;
				case 0xdb:num=10;
					break;
				case 0xbb:num=11;
					break;
				case 0x7b:num=12;
					break;
			}
		while(temp!=0xf0)
			{
				temp=KEY_PORT;
				temp=temp&0xf0;
			}
		}
		Delayms(5); //延时消抖
	}

	KEY_PORT=0xf7;
	temp=KEY_PORT;
	temp=temp&0xf0;
	while(temp!=0xf0)
	{
		Delayms(5); //延时消抖
		temp=KEY_PORT;
		temp=temp&0xf0;
		while(temp!=0xf0)
		{
			temp=KEY_PORT;
		switch(temp)
			{
				case 0xe7:num=13;
					break;
				case 0xd7:num=14;
					break;
				case 0xb7:num=15;
					break;
				case 0x77:num=16;
					break;
			}
		while(temp!=0xf0)
			{
				temp=KEY_PORT;
				temp=temp&0xf0;
			}
		}
		Delayms(5); //延时消抖
	}
return num;
}

/*******************************************************************************
* 函 数 名 ：T1_ISR
* 函数功能 ：定时器1中断服务函数，用于产生38k载频
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void T1_ISR(void) interrupt 3 
{ 
	IR_OUT=!IR_OUT; 
}

/*******************************************************************************
* 函 数 名 ：Exit0Int
* 函数功能 ：外部中断0 ISR
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void Exit0Int() interrupt 0 
{
	EX0 = 0; 
	if(sys_status != READ_RED) return;
	
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;   
	
	while(1)
	{
		if(IRIN == 1)
		{
			while(IRIN)
			{
				if(TH0 > 200)//电平持续时间超出约20ms，即可返回
				{
					if(key_info.count_bit >= 16)
					{
						LED_ok = 0;
						sys_status = READ_OK;
						TR0 = 0;
						return;
					}
					else
					{
						memset(&key_info.key_value , 0 , sizeof(struct KEYMSG));//清空buf
						printf("key_info.count_bit < 16\r\n");
						TR0 = 0;
						EX0 = 1; 
						return;
					}
				}
			}
		}
		else if(IRIN == 0)		
		{
			while(!IRIN);
		}
		TR0 = 0;
		key_info.time[key_info.count_bit*2] = TH0;
		key_info.time[key_info.count_bit*2+1] = TL0;
		
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;//先开始下面的计数的同时执行下面的计算，将误差减到最小
		
		key_info.value_msg[key_info.count_bit/8] = key_info.value_msg[key_info.count_bit/8] | ((unsigned char)(!IRIN) << (7-key_info.count_bit%8));		
		key_info.count_bit++;
		if(key_info.count_bit > 320) 
		{
				memset(&key_info.key_value , 0 , sizeof(struct KEYMSG));//清空buf
				TR0 = 0;
			printf("key_info.count_bit > 160\r\n");
			EX0 = 1; 
				return;			
		}
	}
//     EX0 = 1; 
}

/*******************************************************************************
* 函 数 名 ：beep
* 函数功能 ：蜂鸣器响一声
* 输    入 ：无
* 输    出 ：无
*******************************************************************************/
void beep()
{
  unsigned char i;
  for (i=0;i<180;i++)
   {
     Delayms(1);
     BEEP=!BEEP;                 //BEEP取反
   } 
  BEEP=1;                      //关闭蜂鸣器
}

/*******************************************************************************
* 函 数 名 ：ReadKeyMsg
* 函数功能 ：读取一个键值的信息，键值已经放到key_msg->key_value
* 输    入 ：struct KEYMSG *key_msg 需要加载信息的结构体
* 输    出 ：无
*******************************************************************************/
void ReadKeyMsg(struct KEYMSG *key_msg)
{
	unsigned int index,length;
	unsigned int start_add = (key_msg->key_value-1)*KEY_BLOCK;
	unsigned char *ptemp = (unsigned char *)key_msg;
	
	for(index = 0;index < 43;index++)
	{
		ptemp[index] = X24c16ReadAdd(start_add+index);
		Delayms(1);
	}
	length = key_msg->count_bit*2;//减少读取时间
	for(index = 43;index < length + 43;index++)
	{
		ptemp[index] = X24c16ReadAdd(start_add+index);
		Delayms(1);
	}	
}

/*******************************************************************************
* 函 数 名 ：WriteKeyMsg
* 函数功能 ：写入键值信息到外部存储器
* 输    入 ：struct KEYMSG *key_msg 存放信息的结构体
* 输    出 ：无
*******************************************************************************/
void WriteKeyMsg(struct KEYMSG *key_msg)
{
	unsigned int index,length;
	unsigned int start_add = (key_msg->key_value-1)*KEY_BLOCK;
	unsigned char *ptemp = (unsigned char *)key_msg;
	
	length = key_msg->count_bit*2 + 43;//减少写入时间
	for(index = 0;index < length;index++)
	{
		X24c16WriteAdd(start_add+index,ptemp[index]);
		Delayms(1);
	}
}	

/*******************************************************************************
* 函 数 名 ：SendKeyMsg
* 函数功能 ：发送红外键值,注意发送和接收是反相的，也就是接收为1，则发送0
* 输    入 ：key_msg 记录红外键值信息的结构体
* 输    出 ：无
*******************************************************************************/
void SendKeyMsg(struct KEYMSG *key_msg)
{
	unsigned int count;
	unsigned char flag;
//	unsigned char TH0_temp;
//	unsigned char TL0_temp;
	flag = key_msg->value_msg[0] & (0x01<<7);//先计算，以节约后面时间
	for(count = 0;count < key_msg->count_bit ; count++)
	{
		key_msg->time[count*2] = 256 - key_msg->time[count*2];
		key_msg->time[count*2+1] = 256 - key_msg->time[count*2+1];
	}
	for(count = 0;count < key_msg->count_bit ; count++)
	{
		if(flag == 0)
		{
			TH0 = key_msg->time[count*2];
			TL0 = key_msg->time[count*2+1];
			TR0=1;
			TR1=1; //打开定时器1，在中断中发送载波，即发送1
			
			flag = key_msg->value_msg[(count+1)/8] & (0x01<<(7-((count+1)%8)));//先计算，以节约后面时间		
			
			while(!TF0); 
			
			TR1=0; 
			TF0=0;
			TR0=0; 	
			IR_OUT=1;			
		}
		else
		{
			TH0 = key_msg->time[count*2];
			TL0 = key_msg->time[count*2+1];
			TR0=1; 
	//		TR1=0; //关闭定时器1，发送0
					
			flag = key_msg->value_msg[(count+1)/8] & (0x01<<(7-((count+1)%8)));//先计算，以节约后面时间
			
			while(!TF0); 
			
			TR0=0; 
			TF0=0;	
			IR_OUT=1;
		}
	}
}

void PrintDebugMsg()
{
	int i;
	printf("key_value:%d\r\n",(int)key_info.key_value);
	printf("count_bit:%d\r\n",(int)key_info.count_bit);
	printf("value_msg:");
	for(i=0;i<10;i++)
	{
		printf("%d ",(int)key_info.value_msg[i]);
	}
	printf("\r\n");
	
		printf("time:");
	for(i=0;i<160;i++)
	{
		printf("%d ",(int)key_info.time[i]);
	}
	printf("\r\n");
	printf("\r\n");
}
