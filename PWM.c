//���ڴ��䣬�����ݴ���ɾ����ֱ�Ӵ���ascii
#include "8a8k.h"
#include "delay.h "
#include "intrins.h"
#include <stdio.h>
#include <string.h>

unsigned char flag[2]=0;
unsigned long dir[2] = {0x02ee, 0x02ee};

void PWMInit(void)		//100΢��@24.000MHz
{
	  P2M1 = 0X00;
	  P2M0 = 0X03;
		P_SW2 = 0x80;  //������չSFR
		PWMCKS = 0x0f; // PWM ʱ��Ϊϵͳʱ��  24M/16=1.5M  
		PWMC = 0x7530; //���� PWM ����Ϊ 7530H �� PWM ʱ��  DEC=30000  30000*16/24M=20ms
		PWM0T2= 0x0000; //�ڼ���ֵΪ H �ط�����ߵ�ƽ 
	  PWM0T1= 0x02ee; //�ڼ���ֵΪ H �ط�����͵�ƽ  DEC:750
		PWM1T2= 0x0000; //�ڼ���ֵΪ H �ط�����ߵ�ƽ 
		PWM1T1= 0x02ee; //�ڼ���ֵΪ H �ط�����͵�ƽ 
		PWM0CR= 0x80; //ʹ�� PWM0 ���  ��ʼ��ƽΪ�ߵ�ƽ  P2.0
	  PWM1CR= 0x80; //ʹ�� PWM1 ���  ��ʼ��ƽΪ�ߵ�ƽ  P2.1
		P_SW2 = 0x00; 
		PWMCR = 0xc0; //���� PWM ģ�� ��ʹ���ж�
	  EA = 0;
}
/*
    0.5ms  750  0x02ee
    150��pwm����==0.1ms

    PWM1(�Ϸ����)��
    180��2ms  
		180/9=20��       
    2/20=0.1ms       150��pwm����  0x0096
    ���ޣ� 750+150*(20+1)= 3900    0x0f3c

    PWM2(�·����)��
    270��2ms   187.11  1.386ms
    187.11/8.91=21��
		1.386/21=0.066ms  99��pwm����  0x0063  
		һ����ʵΪ8.91�㣬�����0.09�� ����1.89��
    ���ޣ� 750+99*(21+1)= 2928     0x0b70

*/

void Uart1Init(void)		//115200bps@24.000MHz
{
		SCON = 0x50;		//8λ����,�ɱ䲨����
		AUXR |= 0x40;		//��ʱ��ʱ��1Tģʽ
		AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
		TMOD &= 0x0F;		//���ö�ʱ��ģʽ
		TL1 = 0xCC;		//���ö�ʱ��ʼֵ
		TH1 = 0xFF;		//���ö�ʱ��ʼֵ
		ET1 = 0;		//��ֹ��ʱ��%d�ж�
		TR1 = 1;		//��ʱ��1��ʼ��ʱ
}

void Send_Uart1(unsigned char value) //���ڴ�ӡ����
{  
		ES=0;                 
		TI=0;                 
		SBUF=value;           
		while(TI==0);          
		TI=0;                 
		ES=1;                
}
//ע�⽫��ת�����ַ������ͣ��Ҷ�λ��Ҫ��
void dataManage(unsigned char flag[])
{
		int i;
		unsigned char a=0;
//	 int len,j;
	  unsigned char angle[2][10]=0;
	  //�¶����0.09�㣬ʵΪ8.91��
	  for(i=0;i<2;i++)
		{
			  a=flag[i]*9;
//			 	len = sprintf(angle[i], "%d ", a);//sprintf��8λ2����ת����ʮ�����ַ�����������������У�������ַ����ĳ���
//				for(j = 0; j < len; j++)
//				Send_Uart1(angle[i][j]);//��������txt
//				memset(angle[i], 0, sizeof(angle[i]));//������txt����
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
					PWMCFG &= ~0x80; //���жϱ�־ 
					_push_(P_SW2); 
					P_SW2 |= 0x80;			
			
					dir[0] = dir[0]+0X0096;  //�����ԶԼĴ���ֱ�Ӹ�ֵ
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