#include<reg52.h>

unsigned int n = 62;
unsigned int nbuf = 62;
unsigned char T0RH = 0;
unsigned char T0RL = 0;
unsigned int EX = 2;
unsigned long int totaltime;
unsigned long int x;

void ConfigTimer0(unsigned int ms);
void CountTime();
void TimeWatchCount();
void ShowNumber2(unsigned long int num);
void InitLcd1602();
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);

sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;
#define LCD1602_DB P0
sbit LCD1602_RS = P1^0;
sbit LCD1602_RW = P1^1;
sbit LCD1602_E = P1^5;


sbit KEY_IN_1 = P2^4;
sbit KEY_IN_2 = P2^5;
sbit KEY_IN_3 = P2^6;
sbit KEY_IN_4 = P2^7;
sbit KEY_OUT_1 = P2^3;
sbit KEY_OUT_2 = P2^2;
sbit KEY_OUT_3 = P2^1;
sbit KEY_OUT_4 = P2^0;

unsigned char code image[] = 
{
    0xFF,0xFF,0x81,0x00,0x24,0x24,0x24,0x24, //m
    0xC0,0x80,0x8C,0x8C,0xC0,0xE0,0xC4,0x8C, //R
    0xFF,0xFF,0xFF,0xFB,0xC3,0xB3,0xFB,0xFB, //r
    0xFF,0xFF,0x99,0x99,0x81,0x81,0x99,0x99, //H
    0xFF,0x81,0x81,0xE7,0xE7,0xE7,0xE7,0xE7, //T
};
unsigned char code LedChar[]=
{
	0xC0, 0xF9, 0xA4, 0xB0, 
	0x99, 0x92, 0x82, 0xF8,
	0x80, 0x90, 0x88, 0x83,
	0xC6, 0xA1, 0x86, 0x8E
};
unsigned char LedBuff[6] = 
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

unsigned char KeySta[4][4] = 
{ 
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1}
};
unsigned char code KeyCodeMap[4][4] = 
{ //���󰴼���ŵ���׼���̼����ӳ���
    { 0x31, 0x32, 0x33, 0x26 }, //���ּ�1�����ּ�2�����ּ�3�����ϼ�
    { 0x34, 0x35, 0x36, 0x25 }, //���ּ�4�����ּ�5�����ּ�6�������
    { 0x37, 0x38, 0x39, 0x28 }, //���ּ�7�����ּ�8�����ּ�9�����¼�
    { 0x30, 0x1B, 0x0D, 0x27 }  //���ּ�0��ESC����  �س����� ���Ҽ�
};
void  KeyDriver();

/*������*/
void main()
{
    EA = 1;
    ENLED = 0;    //ѡ������ܽ�����ʾ
    ADDR3 = 1;
    TMOD &= 0xF0;
	TMOD |= 0x01;
    ConfigTimer0(1);
    ET0  = 1;     //ʹ��T0�ж�
    EX0 = 1;      //ʹ�ⲿ�ж�0����
    IT0 = 1;	 //�����ⲿ�жϴ�����ʽ 
    LedBuff[0] = LedChar[((n-2)/2)%10];
    LedBuff[1] = LedChar[((n-2)/2)/10%10];
    while(1)
    {
        KeyDriver(); 
    }
}

/*�������ܺ���*/
void KeyAction(unsigned char keycode)
{
	if((keycode == 0x0d) && EX == 0)
	{
        ShowNumber2(totaltime);
	}
    if(keycode == 0x26)
    {
        if(n < 182)
        {
            n = n + 20;
            nbuf = n;
            LedBuff[0] = LedChar[((n-2)/2)%10];
            LedBuff[1] = LedChar[((n-2)/2)/10%10];
        }
    }
    if(keycode == 0x28)
    {
        if(n > 22)
        {
            n = n - 20;
            nbuf = n;
            LedBuff[0] = LedChar[((n-2)/2)%10];
            LedBuff[1] = LedChar[((n-2)/2)/10%10];
        }
    }
}

/*��ⰴ���Ƿ��µĺ���*/
void  KeyDriver()
{
	unsigned char i, j;
	static	unsigned char backup [4][4] = 
    {
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1}
	};

	for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				if(backup[i][j] != KeySta[i][j])
				{
					if(backup[i][j] == 0)
					{
						KeyAction(KeyCodeMap[i][j]);
					}
					backup[i][j] = KeySta[i][j];
				}
			}	
		}

	
}

/* ����ɨ�躯�������ڶ�ʱ�ж��е��ã��Ƽ����ü��1ms */
void KeyScan()
{
    unsigned char i;
    static unsigned char keyout = 0;   //���󰴼�ɨ���������
    static unsigned char keybuf[4][4] = 
    {  //���󰴼�ɨ�軺����
        {0xFF, 0xFF, 0xFF, 0xFF},  
		{0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF},  
		{0xFF, 0xFF, 0xFF, 0xFF}
    };

    //��һ�е�4������ֵ���뻺����
    keybuf[keyout][0] = (keybuf[keyout][0] << 1) | KEY_IN_1;
    keybuf[keyout][1] = (keybuf[keyout][1] << 1) | KEY_IN_2;
    keybuf[keyout][2] = (keybuf[keyout][2] << 1) | KEY_IN_3;
    keybuf[keyout][3] = (keybuf[keyout][3] << 1) | KEY_IN_4;
    //��������°���״̬
    for (i=0; i<4; i++)  //ÿ��4������������ѭ��4��
    {
        if ((keybuf[keyout][i] & 0x0F) == 0x00)
        {   //����4��ɨ��ֵΪ0����4*4ms�ڶ��ǰ���״̬ʱ������Ϊ�������ȶ��İ���
            KeySta[keyout][i] = 0;
        }
        else if ((keybuf[keyout][i] & 0x0F) == 0x0F)
        {   //����4��ɨ��ֵΪ1����4*4ms�ڶ��ǵ���״̬ʱ������Ϊ�������ȶ��ĵ���
            KeySta[keyout][i] = 1;
        }
    }
    //ִ����һ�ε�ɨ�����
    keyout++;                //�����������
    keyout = keyout & 0x03;  //����ֵ�ӵ�4������
    switch (keyout)          //�����������ͷŵ�ǰ������ţ������´ε��������
    {
        case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
        case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0; break;
        case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
        case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0; break;
        default: break;
    }
}

/* �ȴ�Һ��׼���� */
void LcdWaitReady()
{
 unsigned char sta;

 LCD1602_DB = 0xFF;
 LCD1602_RS = 0;
 LCD1602_RW = 1;
 do {
 LCD1602_E = 1;
 sta = LCD1602_DB; //��ȡ״̬��
 LCD1602_E = 0;
 } while (sta & 0x80); //bit7 ���� 1 ��ʾҺ����æ���ظ����ֱ������� 0 Ϊֹ
}
/* �� LCD1602 Һ��д��һ�ֽ����cmd-��д������ֵ */
void LcdWriteCmd(unsigned char cmd)
{
 LcdWaitReady();
 LCD1602_RS = 0;
 LCD1602_RW = 0;
 LCD1602_DB = cmd;
 LCD1602_E = 1;
 LCD1602_E = 0;
}
/* �� LCD1602 Һ��д��һ�ֽ����ݣ�dat-��д������ֵ */
void LcdWriteDat(unsigned char dat)
{
 LcdWaitReady();
 LCD1602_RS = 1;
 LCD1602_RW = 0;
 LCD1602_DB = dat;
 LCD1602_E = 1;
 LCD1602_E = 0;
}
/* ������ʾ RAM ��ʼ��ַ���༴���λ�ã�(x,y)-��Ӧ��Ļ�ϵ��ַ����� */
void LcdSetCursor(unsigned char x, unsigned char y)
{
 unsigned char addr;

 if (y == 0) //���������Ļ���������ʾ RAM �ĵ�ַ
 addr = 0x00 + x; //��һ���ַ���ַ�� 0x00 ��ʼ
 else
 addr = 0x40 + x; //�ڶ����ַ���ַ�� 0x40 ��ʼ
 LcdWriteCmd(addr | 0x80); //���� RAM ��ַ
}
/* ��Һ������ʾ�ַ�����(x,y)-��Ӧ��Ļ�ϵ���ʼ���꣬str-�ַ���ָ�� */
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str)
{
 LcdSetCursor(x, y); //������ʼ��ַ
 while (*str != '\0') //����д���ַ������ݣ�ֱ����⵽������
 {
 LcdWriteDat(*str++); //��ȡ str ָ������ݣ�Ȼ�� str �Լ� 1
 }

}
/* ��ʼ�� 1602 Һ�� */
void InitLcd1602()
{
 LcdWriteCmd(0x38); //16*2 ��ʾ��5*7 ����8 λ���ݽӿ�
 LcdWriteCmd(0x0C); //��ʾ���������ر�
 LcdWriteCmd(0x06); //���ֲ�������ַ�Զ�+1
 LcdWriteCmd(0x01); //����
}


/*����ܸ�ֵ����*/
void ShowNumber1()
{
    LedBuff[0] = LedChar[x%10];
    LedBuff[1] = LedChar[x/10%10];
}

/*����ܸ�ֵ����*/
void ShowNumber2(unsigned long int num)
{
    signed char j;
	unsigned char buf[6];

	for(j=0; j<6; j++)
	{
		buf[j] = num % 10;
		num = num / 10;
	}

	for(j=5; j>=3; j--)
	{
		if(buf[j] == 0)
		{
			LedBuff[j] = 0xFF;
		}
		else
			break;
	}
	
	for(; j>=0; j--)
	{
		LedBuff[j] = LedChar[buf[j]];	
	}
    LedBuff[2] &= 0x7F;
}

/* ����ܶ�̬ɨ��ˢ�º��������ڶ�ʱ�ж��е��� */
void LedScan()
{
    static unsigned char i = 0;  //��̬ɨ�������
    
    P0 = 0xFF;   //��ʾ����

    P1 = ((P1 & 0xf8) | i);
    P0 = LedBuff[i];
    if(i < 5)
      i++;
    else
      i = 0;
}

/*���ڽ����ر��ⲿ�ж�*/
void CountTime()
{
    if(n <= 1)
    {
        EX0 = 0;
        EX = 0;
    }
}

/*���ڼ��㶨ʱ����ֵ�ĺ���*/
void ConfigTimer0(unsigned int ms)
{
	unsigned long tmp;

	tmp = 11059200/12;
	tmp = (tmp * ms)/1000;
	tmp = 65536 - tmp;
	tmp = tmp + 18;

	T0RH = (unsigned char)(tmp >> 8);
	T0RL = (unsigned char)tmp;
	TMOD &= 0xF0;
	TMOD |= 0x01;
	TH0 = T0RH;
	TL0 = T0RL;
	ET0 = 1;
    TR0 = 1;
}

/*�������ʾ������*/
void PeriodWatch()
{
    if(n > nbuf - 3)
    {
        x = (nbuf-2)/2;
    }
    if(n == nbuf - 3)
    {
        x = (nbuf-2)/2 - 1;
    }
    if((n < nbuf - 3) && ((n-1)%2 == 0))
    {
        x = (n - 1) / 2;
    }
    ShowNumber1();
}

/*���뺯��*/
void TimewatchCount()
{
    totaltime++;
}

/*�ⲿ�ж�0�ĺ���*/
void interruptEX0() interrupt 0  //�ⲿ�жϣ�������������Ƿ���ס
{
    EX = 1;
    n--;
    CountTime();
    PeriodWatch(); 
}

/*��ʱ��0���жϺ���*/
void interrupTimer0() interrupt 1
{
    static unsigned long int tmr10ms;
    TH0 = T0RH;  //���¼�������ֵ
    TL0 = T0RL;
    LedScan();   //�����ɨ��
    KeyScan();   //����ɨ��
    if(EX == 1)
    {
        tmr10ms++;
        if(tmr10ms >= 10)
        {
        tmr10ms = 0;
        TimeWatchCount();
        }
    }    
}