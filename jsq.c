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
 * �ӳٺ���
 * 11.0592MHz�����£��ӳ�Լ1ms
******/
void delayms(uint ms) 
{
	uchar i;
	while (ms--)
		for (i = 0; i < 123; i++);
}

/******
 * д�����
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
 * д������
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
 * д���ַ�
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
 * д���ַ���
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
 * Һ������ʼ��
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
 * 4x4���󰴼�ɨ��
 * ʹ�ð���ɨ�跨
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
 * �������������
******/
uchar key_pro()
{
	switch(key_scan())
	{
		case 0x7e : return '/'; break;	//����0
		case 0x7d : return 'x'; break;	//����1
		case 0x7b : return '-'; break;	//����2
		case 0x77 : return '+'; break;	//����3
		case 0xbe : return '='; break;	//����4
		case 0xbd : return '3'; break;	//����5
		case 0xbb : return '6'; break;	//����6
		case 0xb7 : return '9'; break;	//����7
		case 0xde : return '.'; break;	//����8
		case 0xdd : return '2'; break;	//����9
		case 0xdb : return '5'; break;	//����A
		case 0xd7 : return '8'; break;	//����B
		case 0xee : return '0'; break;	//����C
		case 0xed : return '1'; break;	//����D
		case 0xeb : return '4'; break;	//����E
		case 0xe7  :return '7'; break;	//����F
		default : return 0xff; break;
	}
}

/******
 * ������
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
		if(num != 0xff)         //���ɨ����Чֵ�������һ��
		{
			if(i == 0)            //�����һ���ַ���ʱ����Ҫ�Ѻ������
				w_com(0x01);
			if(('+' == num) || (i == 16) || ('-' == num) || ('x' == num) || ('/' == num) || ('=' == num))
			{
				i = 0;              //��������λ
				if(firstflag == 0)//flag����0����˵��֮ǰû��������֣���������һ��������
        		{
					sscanf(temp, "%f", &a);//���뱻����
					firstflag = 1;
				}
				else
					sscanf(temp, "%f", &b);//���flag����1����֮ǰ������һ�����ӻ�����ˡ�����
				for(s = 0; s < 16; s++) //����������
				{
					temp[s] = 0;
				}
				w_Char(0, 1, num);  //�����ڵڶ���
				if(num != '=')
					sign = num;       //�������Ĳ��ǵȺţ����±�־λ
				else
				{
					firstflag = 0;    //��⵽����'='�ţ��ж��ϴζ���ķ���
					switch(sign)
					{
						case '+': a = a + b; break;
						case '-': a = a - b; break;
						case 'x': a = a * b; break;
						case '/': a = a / b; break;
						default: break;
					}
					sprintf(temp, "%g", a);  //���������
					w_str(1, 1, temp);       //��ʾ��Һ����
					sign = 0;    //֮����������
					a = b = 0;   //֮����������            
					for(s = 0; s < 16; s++)
					temp[s] = 0;
				}
			}
			else if(i < 16)
			{
				if((1 == i) && (temp[0] == '0'))  //�����һ���ַ�Ϊ0�������һ���ַ������ж�
				{
					if(num == '.')       //�����С���㣬����λ�ü�1
					{
						temp[1] = '.';
						w_Char(1, 0, num);   //�������
						i++;
					}
					else
					{
						temp[0] = num;      //���������1-9����˵��0û�ã����滻0���ڵĵ�һλ
						w_Char(0, 0, num);  //�������
					}
				}
				else
				{
					temp[i] = num;
					w_Char(i, 0, num);   //�������
					i++;               //������ֵ�ۼ�
				}
			}
		}
	}
}
