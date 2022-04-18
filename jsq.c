#include<reg52.h>
#include<stdio.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned char

sbit RS = P2^4;
sbit RW = P2^5;
sbit E = P2^6;
bit kp;

/******
 * 延迟函数
 * 11.0592MHz晶振下，延迟约1ms
******/
void delayms(uint ms) 
{
	uchar i;
	while (ms--)
		for (i = 0; i < 123; i++);
}

/******
 * 写命令函数
******/
void w_com(uchar com)
{
	RS = 0;
	RW = 0;
	E = 1;
	P0 = com;
	E = 0;
	delayms(1);
}

/******
 * 写入数据
******/
void w_dat(uchar dat)
{
	RS = 1;
	RW = 0;
	E = 1;
	P0 = dat;
	E = 0;
	delayms(1);
}

/******
 * 写入字符
******/
void w_Char(uchar x,uchar y,uchar dat)
{
	if(y == 0)
	{
		w_com(0x80 + x);
	}
	else
	{
		w_com(0xc0 + x);
	}
	w_dat(dat);
}

/******
 * 写入字符串
******/
void w_str(uchar x, uchar y, uchar *s)
{
	while(*s)
	{
		w_Char(x, y, *s);
		s++;
		x++;
	}
}

/******
 * 液晶屏初始化
******/
void LCD_init()
{
	delayms(10);
	w_com(0x38);
	delayms(10);
	w_com(0x06);
	delayms(10);
	w_com(0x0c);
	delayms(10);
	w_com(0x01);
	delayms(10);
	w_com(0x38);
	delayms(10);
}

/******
 * 4x4矩阵按键扫描
 * 使用按键扫描法
******/
uchar key_scan()
{
	uchar scan1, scan2, temp;
	static bit kp =0 ;
	P3 = 0xf0;
	scan1 = P3 & 0xf0;
	if(scan1 != 0xf0)
	{
		delayms(10);
		scan1 = P3 & 0xf0;
		if((scan1 != 0xf0) && (kp == 0))
		{
			kp = 1;
			P3 = 0x0f;
			scan2 = P3 % 0x0f;
			temp = scan1 | scan2;
			return temp;
		}
	}
	else 
		kp = 0;
	return 0xff;
}

/******
 * 定义各按键含义
******/
uchar key_pro()
{
	switch(key_scan())
	{
		case 0x7e : return '/'; break;	//按键0
		case 0x7d : return 'x'; break;	//按键1
		case 0x7b : return '-'; break;	//按键2
		case 0x77 : return '+'; break;	//按键3
		case 0xbe : return '='; break;	//按键4
		case 0xbd : return '3'; break;	//按键5
		case 0xbb : return '6'; break;	//按键6
		case 0xb7 : return '9'; break;	//按键7
		case 0xde : return '.'; break;	//按键8
		case 0xdd : return '2'; break;	//按键9
		case 0xdb : return '5'; break;	//按键A
		case 0xd7 : return '8'; break;	//按键B
		case 0xee : return '0'; break;	//按键C
		case 0xed : return '1'; break;	//按键D
		case 0xeb : return '4'; break;	//按键E
		case 0xe7  :return '7'; break;	//按键F
		default : return 0xff; break;
	}
}

/******
 * 主函数
******/
int main(void)
{
	uchar num, i, sign;
	uchar temp[16];
	bit firstflag;
	float a = 0, b = 0;
	uchar s;

	LCD_init();
	delayms(10);
	w_com(0x01);
	delayms(200);
	w_com(0x01);
	while(1)
	{
		num = key_pro();
		if(num != 0xff)         //如果扫描有效值则进入下一步
		{
			if(i == 0)            //输入第一个字符的时，需要把后面清空
				w_com(0x01);
			if(('+' == num) || (i == 16) || ('-' == num) || ('x' == num) || ('/' == num) || ('=' == num))
			{
				i = 0;              //计算器复位
				if(firstflag == 0)//flag等于0，则说明之前没书如果数字，现在输入一个被加数
        		{
					sscanf(temp, "%f", &a);//输入被加数
					firstflag = 1;
				}
				else
					sscanf(temp, "%f", &b);//如果flag等于1，则之前输入了一个被加或减、乘、除数
				for(s = 0; s < 16; s++) //缓冲区清理
				{
					temp[s] = 0;
				}
				w_Char(0, 1, num);  //符号在第二行
				if(num != '=')
					sign = num;       //如果输入的不是等号，记下标志位
				else
				{
					firstflag = 0;    //检测到输入'='号，判断上次读入的符号
					switch(sign)
					{
						case '+': a = a + b; break;
						case '-': a = a - b; break;
						case 'x': a = a * b; break;
						case '/': a = a / b; break;
						default: break;
					}
					sprintf(temp, "%g", a);  //输出浮点型
					w_str(1, 1, temp);       //显示到液晶屏
					sign = 0;    //之后数据清零
					a = b = 0;   //之后数据清零            
					for(s = 0; s < 16; s++)
					temp[s] = 0;
				}
			}
			else if(i < 16)
			{
				if((1 == i) && (temp[0] == '0'))  //如果第一个字符为0，则对下一个字符进行判断
				{
					if(num == '.')       //如果是小数点，则光标位置加1
					{
						temp[1] = '.';
						w_Char(1, 0, num);   //输出数据
						i++;
					}
					else
					{
						temp[0] = num;      //如果是数字1-9，则说明0没用，则替换0所在的第一位
						w_Char(0, 0, num);  //输出数据
					}
				}
				else
				{
					temp[i] = num;
					w_Char(i, 0, num);   //输出数据
					i++;               //输入数值累加
				}
			}
		}
	}
}
