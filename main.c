#include <STC15F2K60S2.H>
#include "onewire.h"
#include "ds1302.h"

typedef unsigned char uchar;
typedef unsigned int uint;

sbit S7 = P3^0;
sbit S6 = P3^1;
sbit S5 = P3^2;
sbit S4 = P3^3;
sbit L1 = P0^0;

uchar code dpnum[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
uchar code Time_Wadd[]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
uchar code Time_Radd[]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
uchar code Time_Init[]={0x50,0x59,0x23,0x01,0x04,0x05,0x22};
uchar Time_save[7]={0};
uint temperature_save[10]={0};
uchar tempnum=0;
uchar count_t;
uint count_t1;
uchar Collect_interval=1;//�ɼ����
uchar Collect_interval_state=0;
uchar Interface=0;
bit smgstate=0;
bit tempflag=0;
uint temperature;
uchar index=0;

void System_Init();
void ColTset();
void display_bit(uchar pos,uchar dat);
void delayms(uchar ms);
void Face();
void key();
void Time_Reset();
void Time_Read();
void Timer0_Init();
void display_interface2();
void delay_L(uchar ms);
void Timer1_Init();
void Tsave();
void display_Index(uchar index);
uint DS18B20_Read();

void main()
{
	System_Init();
	Timer0_Init();
	Timer1_Init();
	Time_Reset();
	while(1)
	{
		temperature= DS18B20_Read();//�ȴ�DS18B20��������,��������һ������Ϊ00���¶�������һ�βɼ�������Ϊ09���¶�ֵ������һ�¸�ֵ�����Ҳû���⣬�����²�
		                             //Ӧ���ǵ��ɼ���10������ʱ��ʱ���رգ�Ȼ�����һֱ��û�м���¶ȵĳ����ٴδ򿪶�ʱ��1�Ͷ����ϴιرն�ʱ��1ʱ����¶�ֵ
		                             //������仰��ʱ��1�رպ��ܼ���ˢ���¶ȵ�ֵ��Ȼ������Ϊ00���¶Ȳɼ�����
		Time_Read();
		key();
		ColTset();
		Face();
	}
}

void System_Init()
{
	P2 = (P2 & 0x1f) | 0x80;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xa0;
	P0 = 0x00;
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}
void ColTset()
{
	switch(Collect_interval_state)
	{
		case 0:
			Collect_interval=1;
			break;
		case 1:
			Collect_interval=5;
			break;
		case 2:
			Collect_interval=30;
			break;
		case 3:
			Collect_interval=60;
			break;
	}
}
void display_bit(uchar pos,uchar dat)
{
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0x01 << pos;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = dat;
}
void delayms(uchar ms)
{
	uint i,j;
	for(i=ms;i>0;i--)
		for(j=810;j>0;j--);
}
void Face()
{
	switch(Interface)
	{
		case 0:
			display_bit(5,dpnum[10]);
			delayms(1);
			display_bit(6,dpnum[Collect_interval/10]);
			delayms(1);
			display_bit(7,dpnum[Collect_interval%10]);
			delayms(1);
			P2 = (P2 & 0x1f) | 0xc0;
			P0 = 0xff;
			P2 = (P2 & 0x1f) | 0xe0;
			P0 = 0xff;
			break;
		case 1:  //ʱ����ʾ����
			TR0=1;  //�򿪶�ʱ��0
			display_interface2();
			TR1 = 1;//��ʼ��ʱ
			Tsave();
			if(S5==0)
			{}  //�ڽ���1����S5��Ч
			while(TR1==0)
			{
				P2 = (P2 & 0x1f)| 0x80;
				P0 |=0XFE;
				L1 = 0;
				delay_L(5);
				P2 = (P2 & 0x1f)| 0x80;
				P0 |=0XFE;
				L1 = 1;
				delay_L(5);
				if(S6==0||S7==0)
				{
					delayms(5);
					if(S6==0||S7==0)
						break;  //������S6��S7����ʱ����ѭ��
				}
			}
			display_bit(0,dpnum[Time_save[2]/16]);
			delayms(1);
			display_bit(1,dpnum[Time_save[2]%16]);
			delayms(1);
//			display_bit(2,dpnum[10]);
//			delayms(1);
			display_bit(3,dpnum[Time_save[1]/16]);
			delayms(1);
			display_bit(4,dpnum[Time_save[1]%16]);
			delayms(1);
//			display_bit(5,dpnum[10]);
//			delayms(1);     //�����жϺ������棬�ﵽ��˸����
			display_bit(6,dpnum[Time_save[0]/16]);
			delayms(1);
			display_bit(7,dpnum[Time_save[0]%16]);
			delayms(1);
			P2 = (P2 & 0x1f) | 0xc0;
			P0 = 0xff;
			P2 = (P2 & 0x1f) | 0xe0;
			P0 = 0xff;
			break;
		case 2:
			display_Index(index);
			break;
	}
}
void delay_L(uchar ms)
{
	uint i,j;
	for(i=ms;i>0;i--)
	{
		for(j=10;j>0;j--)
		{
			display_Index(0);//�Զ���ʾ��һ�βɼ�������
		}
	}
}
void key()
{
	if(S4==0 && Interface==0) //����S4�ڽ���0��Ч
	{
		delayms(5);
		if(S4==0)
		{
			Collect_interval_state++;
			if(Collect_interval_state>3)
			{
				Collect_interval_state=0;
			}
			while(S4==0);
		}
	}
	else if(S5==0)
	{
		delayms(5);
		if(S5==0)
		{
			Interface=1;
		}
	}
	else if(S6==0)
	{
		delayms(5);
		if(S6==0)
		{
			Interface=2;//����S6�������2
			P2 = (P2 & 0x1f)| 0x80;
			P0 |=0X00;
			index++;
			if(index>9)
				index=0;
			while(S6==0);
		}
	}
	else if(S7==0)
	{
		uchar i;
		delayms(5);
		if(S7==0)
		{
			Interface=0;
			tempnum=0;  //ʵ���ڲɼ��¶ȵ�û������ʱ�򣬰���S7�ܹ���Ϊ��ǰ�����ɼ�
			for(i=0;i<10;i++)
			{
				temperature_save[i]=0; //����S7��ŵ�10���¶�����ȫ����0����ʼ��һ�ּ��
			}
	  }
	}
}
void Time_Reset()//ʱ���ʼ��
{
	uchar i;
	Write_Ds1302_Byte(0x8e,0x00);  //00����������Ĵ�����д������
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(Time_Wadd[i],Time_Init[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);  //80�����ֹ��Ĵ�����д������
}
void Time_Read() //ʱ���ȡ
{
	uchar i;
	for(i=0;i<7;i++)
	{
		Time_save[i]=Read_Ds1302_Byte(Time_Radd[i]);
	}
}
void Timer0_Init()
{
	TMOD |= 0X01;
	TH0 = (65536-10000)/256;
	TL0 = (65536-10000)%256;
	//TR0 = 1;
	ET0 = 1;
	EA = 1;
}
void Timer0() interrupt 1
{
	TH0 = (65536-10000)/256;
	TL0 = (65536-10000)%256;
	count_t++;
	if(count_t==50)
	{
		count_t=0;
		smgstate=~smgstate;
	}
}
void display_interface2()
{
	if(smgstate==1)
	{
		P2 = (P2 & 0x1f) | 0xc0;
		P0 = 0x24;
		P2 = (P2 & 0x1f) | 0xe0;
		P0 = 0xbf;
		delayms(1);//�������������
	}
	else if(smgstate==0)
	{
		P2 = (P2 & 0x1f) | 0xc0;
		P0 = 0x24;
		P2 = (P2 & 0x1f) | 0xe0;
		P0 = 0xff;
		delayms(1);
	}
}
uint DS18B20_Read()
{
	uchar LSB,MSB;
	uint temp;//��ʾ�¶ȵı�����Ӧ��Ϊint��
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	delayms(1);
	
	LSB = Read_DS18B20();
	MSB = Read_DS18B20();
	
	temp = (MSB << 8) | LSB;
	temp >>= 4;
	return temp;
}
void Timer1_Init()
{
	TMOD |= 0X00;
	TH1 = (65536-10000)/256;
	TL1 = (65536-10000)%256;
	TR1 = 0;
	ET1 = 1;
	EA = 1;
}
void Timer1() interrupt 3
{
	count_t1++;
	switch(Collect_interval)
	{
		case 1:
			if(count_t1==100)
			{
				count_t1=0;
				tempflag=1;//��ȡ�¶�����
			}
			break;
		case 5:
			if(count_t1==500)
			{
				count_t1=0;
				tempflag=1;
			}
			break;
		case 30:
			if(count_t1==3000)
			{
				count_t1=0;
				tempflag=1;
			}
			break;
		case 60:
			if(count_t1==6000)
			{
				count_t1=0;
				tempflag=1;
			}
			break;
	}
}
void Tsave()
{
	if(tempflag==1)
	{
		temperature_save[tempnum]=DS18B20_Read();
		tempnum++;
		tempflag=0;
	}
	if(tempnum>9)
	{
		TR1 = 0;//ֹͣ��ȡ�¶�
		tempnum=0; //һ���¶Ȳɼ�����ʱ�������Ҫ���Ĳɼ�������ٴΰ���S5�������1���²ɼ������Բ���S7
	}
}
void display_Index(uchar index) 
{
	display_bit(0,dpnum[10]);
	delayms(1);
	display_bit(1,dpnum[index/10]);
	delayms(1);
	display_bit(2,dpnum[index%10]);
	delayms(1);
	display_bit(3,dpnum[11]);
	delayms(1);
	display_bit(4,dpnum[11]);
	delayms(1);
	display_bit(5,dpnum[10]);
	delayms(1);
	display_bit(6,dpnum[temperature_save[index]/10]);
	delayms(1);
	display_bit(7,dpnum[temperature_save[index]%10]);
	delayms(1);
	P2 = (P2 & 0x1f) | 0xc0;
	P0 = 0xff;
	P2 = (P2 & 0x1f) | 0xe0;
	P0 = 0xff;
}