//用于传输，把数据处理删除，直接传输ascii
#include "8a8k.h"
#include "delay.h "
#include "intrins.h"
#include <stdio.h>
#include <string.h>

unsigned char flag[2]=0;
unsigned long dir[2] = {0x02ee, 0x02ee};

void PWMInit(void)		//100微秒@24.000MHz
{
	  P2M1 = 0X00;
	  P2M0 = 0X03;
		P_SW2 = 0x80;  //访问扩展SFR
		PWMCKS = 0x0f; // PWM 时钟为系统时钟  24M/16=1.5M  
		PWMC = 0x7530; //设置 PWM 周期为 7530H 个 PWM 时钟  DEC=30000  30000*16/24M=20ms
		PWM0T2= 0x0000; //在计数值为 H 地方输出高电平 
	  PWM0T1= 0x02ee; //在计数值为 H 地方输出低电平  DEC:750
		PWM1T2= 0x0000; //在计数值为 H 地方输出高电平 
		PWM1T1= 0x02ee; //在计数值为 H 地方输出低电平 
		PWM0CR= 0x80; //使能 PWM0 输出  初始电平为高电平  P2.0
	  PWM1CR= 0x80; //使能 PWM1 输出  初始电平为高电平  P2.1
		P_SW2 = 0x00; 
		PWMCR = 0xc0; //启动 PWM 模块 并使能中断
	  EA = 0;
}
/*
    0.5ms  750  0x02ee
    150个pwm周期==0.1ms

    PWM1(上方舵机)：
    180°2ms  
		180/9=20个       
    2/20=0.1ms       150个pwm周期  0x0096
    门限： 750+150*(20+1)= 3900    0x0f3c

    PWM2(下方舵机)：
    270°2ms   187.11  1.386ms
    187.11/8.91=21个
		1.386/21=0.066ms  99个pwm周期  0x0063  
		一个点实为8.91°，传输多0.09° 共多1.89°
    门限： 750+99*(21+1)= 2928     0x0b70

*/

void Uart1Init(void)		//115200bps@24.000MHz
{
		SCON = 0x50;		//8位数据,可变波特率
		AUXR |= 0x40;		//定时器时钟1T模式
		AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
		TMOD &= 0x0F;		//设置定时器模式
		TL1 = 0xCC;		//设置定时初始值
		TH1 = 0xFF;		//设置定时初始值
		ET1 = 0;		//禁止定时器%d中断
		TR1 = 1;		//定时器1开始计时
}

void Send_Uart1(unsigned char value) //串口打印发送
{  
		ES=0;                 
		TI=0;                 
		SBUF=value;           
		while(TI==0);          
		TI=0;                 
		ES=1;                
}
//注意将数转换成字符串发送，且多位数要拆开
void dataManage(unsigned char flag[])
{
		int i;
		unsigned char a=0;
//	 int len,j;
	  unsigned char angle[2][10]=0;
	  //下舵机多0.09°，实为8.91°
	  for(i=0;i<2;i++)
		{
			  a=flag[i]*9;
//			 	len = sprintf(angle[i], "%d ", a);//sprintf将8位2进制转换成十进制字符串，把其放入数组中，并输出字符串的长度
//				for(j = 0; j < len; j++)
//				Send_Uart1(angle[i][j]);//发送数组txt
//				memset(angle[i], 0, sizeof(angle[i]));//把数组txt清零
			  Send_Uart1(a);
		}
		Send_Uart1('\n');

}

void main()
{
	  PWMInit();
	  Uart1Init();
    delay_s(10);
		while(1)
		{ 
				Delay500ms();
	      EA = 1;
			  dataManage(flag);
		}
}

void PWM_Isr() interrupt 22 
{ 
		if (PWMCFG & 0x80) 
		{ 
					PWMCFG &= ~0x80; //清中断标志 
					_push_(P_SW2); 
					P_SW2 |= 0x80;			
			
					dir[0] = dir[0]+0X0096;  //不可以对寄存器直接赋值
			    flag[0]++;
       		PWM0T1 = dir[0];	
					if(dir[0]>= 0x0f3c)//750+150*(20+1)
					{
							dir[0] = 0x02ee;
						  flag[0]=0;
						  PWM0T1 = dir[0];
							dir[1] = dir[1]+0X0063;
						  flag[1]++;
						  PWM1T1 = dir[1];
							if(dir[1]>= 0x0b70)//750+99*(21+1)= 2928
							{
									dir[1] = 0x02ee;
								  flag[1]=0;
									PWM1T1 = dir[1];								
							}
					}		
					_pop_(P_SW2); 
		}
	  EA = 0;		
}