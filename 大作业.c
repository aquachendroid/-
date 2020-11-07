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
{ //矩阵按键编号到标准键盘键码的映射表
    { 0x31, 0x32, 0x33, 0x26 }, //数字键1、数字键2、数字键3、向上键
    { 0x34, 0x35, 0x36, 0x25 }, //数字键4、数字键5、数字键6、向左键
    { 0x37, 0x38, 0x39, 0x28 }, //数字键7、数字键8、数字键9、向下键
    { 0x30, 0x1B, 0x0D, 0x27 }  //数字键0、ESC键、  回车键、 向右键
};
void  KeyDriver();

/*主函数*/
void main()
{
    EA = 1;
    ENLED = 0;    //选择数码管进行显示
    ADDR3 = 1;
    TMOD &= 0xF0;
	TMOD |= 0x01;
    ConfigTimer0(1);
    ET0  = 1;     //使能T0中断
    EX0 = 1;      //使外部中断0工作
    IT0 = 1;	 //设置外部中断触发方式 
    LedBuff[0] = LedChar[((n-2)/2)%10];
    LedBuff[1] = LedChar[((n-2)/2)/10%10];
    while(1)
    {
        KeyDriver(); 
    
        
    }
}

/*按键功能函数*/
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

/*监测按键是否按下的函数*/
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

/* 按键扫描函数，需在定时中断中调用，推荐调用间隔1ms */
void KeyScan()
{
    unsigned char i;
    static unsigned char keyout = 0;   //矩阵按键扫描输出索引
    static unsigned char keybuf[4][4] = 
    {  //矩阵按键扫描缓冲区
        {0xFF, 0xFF, 0xFF, 0xFF},  
		{0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF},  
		{0xFF, 0xFF, 0xFF, 0xFF}
    };

    //将一行的4个按键值移入缓冲区
    keybuf[keyout][0] = (keybuf[keyout][0] << 1) | KEY_IN_1;
    keybuf[keyout][1] = (keybuf[keyout][1] << 1) | KEY_IN_2;
    keybuf[keyout][2] = (keybuf[keyout][2] << 1) | KEY_IN_3;
    keybuf[keyout][3] = (keybuf[keyout][3] << 1) | KEY_IN_4;
    //消抖后更新按键状态
    for (i=0; i<4; i++)  //每行4个按键，所以循环4次
    {
        if ((keybuf[keyout][i] & 0x0F) == 0x00)
        {   //连续4次扫描值为0，即4*4ms内都是按下状态时，可认为按键已稳定的按下
            KeySta[keyout][i] = 0;
        }
        else if ((keybuf[keyout][i] & 0x0F) == 0x0F)
        {   //连续4次扫描值为1，即4*4ms内都是弹起状态时，可认为按键已稳定的弹起
            KeySta[keyout][i] = 1;
        }
    }
    //执行下一次的扫描输出
    keyout++;                //输出索引递增
    keyout = keyout & 0x03;  //索引值加到4即归零
    switch (keyout)          //根据索引，释放当前输出引脚，拉低下次的输出引脚
    {
        case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
        case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0; break;
        case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
        case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0; break;
        default: break;
    }
}

/* 等待液晶准备好 */
void LcdWaitReady()
{
 unsigned char sta;

 LCD1602_DB = 0xFF;
 LCD1602_RS = 0;
 LCD1602_RW = 1;
 do {
 LCD1602_E = 1;
 sta = LCD1602_DB; //读取状态字
 LCD1602_E = 0;
 } while (sta & 0x80); //bit7 等于 1 表示液晶正忙，重复检测直到其等于 0 为止
}
/* 向 LCD1602 液晶写入一字节命令，cmd-待写入命令值 */
void LcdWriteCmd(unsigned char cmd)
{
 LcdWaitReady();
 LCD1602_RS = 0;
 LCD1602_RW = 0;
 LCD1602_DB = cmd;
 LCD1602_E = 1;
 LCD1602_E = 0;
}
/* 向 LCD1602 液晶写入一字节数据，dat-待写入数据值 */
void LcdWriteDat(unsigned char dat)
{
 LcdWaitReady();
 LCD1602_RS = 1;
 LCD1602_RW = 0;
 LCD1602_DB = dat;
 LCD1602_E = 1;
 LCD1602_E = 0;
}
/* 设置显示 RAM 起始地址，亦即光标位置，(x,y)-对应屏幕上的字符坐标 */
void LcdSetCursor(unsigned char x, unsigned char y)
{
 unsigned char addr;

 if (y == 0) //由输入的屏幕坐标计算显示 RAM 的地址
 addr = 0x00 + x; //第一行字符地址从 0x00 起始
 else
 addr = 0x40 + x; //第二行字符地址从 0x40 起始
 LcdWriteCmd(addr | 0x80); //设置 RAM 地址
}
/* 在液晶上显示字符串，(x,y)-对应屏幕上的起始坐标，str-字符串指针 */
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str)
{
 LcdSetCursor(x, y); //设置起始地址
 while (*str != '\0') //连续写入字符串数据，直到检测到结束符
 {
 LcdWriteDat(*str++); //先取 str 指向的数据，然后 str 自加 1
 }

}
/* 初始化 1602 液晶 */
void InitLcd1602()
{
 LcdWriteCmd(0x38); //16*2 显示，5*7 点阵，8 位数据接口
 LcdWriteCmd(0x0C); //显示器开，光标关闭
 LcdWriteCmd(0x06); //文字不动，地址自动+1
 LcdWriteCmd(0x01); //清屏
}


/*数码管赋值函数*/
void ShowNumber1()
{
    LedBuff[0] = LedChar[x%10];
    LedBuff[1] = LedChar[x/10%10];
}

/*数码管赋值函数*/
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

/* 数码管动态扫描刷新函数，需在定时中断中调用 */
void LedScan()
{
    static unsigned char i = 0;  //动态扫描的索引
    
    P0 = 0xFF;   //显示消隐

    P1 = ((P1 & 0xf8) | i);
    P0 = LedBuff[i];
    if(i < 5)
      i++;
    else
      i = 0;
}

/*周期结束关闭外部中断*/
void CountTime()
{
    if(n <= 1)
    {
        EX0 = 0;
        EX = 0;
    }
}

/*用于计算定时器初值的函数*/
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

/*数码管显示周期数*/
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

/*读秒函数*/
void TimewatchCount()
{
    totaltime++;
}

/*外部中断0的函数*/
void interruptEX0() interrupt 0  //外部中断，用来检测光电门是否被遮住
{
    EX = 1;
    n--;
    CountTime();
    PeriodWatch(); 
}

/*定时器0的中断函数*/
void interrupTimer0() interrupt 1
{
    static unsigned long int tmr10ms;
    TH0 = T0RH;  //重新加载重载值
    TL0 = T0RL;
    LedScan();   //数码管扫描
    KeyScan();   //按键扫描
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