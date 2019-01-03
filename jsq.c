#include<reg52.h>
#include<stdio.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned char
sbit RS=P2^4;
sbit RW=P2^5;
sbit E=P2^6;
bit kp;
void delayms(uint ms)
{
	uchar i;
	while(ms--)
		for(i=0;i<123;i++);
}
void w_com(uchar com) //Ð´ÈëÃüÁî
{
	RS=0;
	RW=0;
	E=1;
	P0=com;
	E=0;
	delayms(1);
}
void w_dat(uchar dat) //Ð´ÈëÊý¾Ý
{
	RS=1;
	RW=0;
	E=1;
	P0=dat;
	E=0;
	delayms(1);
}
void w_Char(uchar x,uchar y,uchar dat) //Ð´Èë×Ö·û
{
	if(y==0)
	{
		w_com(0x80+x);
	}
	else
	{
		w_com(0xc0+x);
	}
	w_dat(dat);
}
void w_str(uchar x,uchar y,uchar *s) //Ð´Èë×Ö·û´®
{
	while(*s)
	{
		w_Char(x,y,*s);
		s++;
		x++;
	}
}
void LCD_init() //Òº¾§ÆÁ³õÊ¼»¯
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
uchar key_scan()   //°´¼üÉ¨Ãè·¨½øÐÐ¾ØÕó°´¼üÉ¨Ãè
{
	uchar scan1,scan2,temp;
	static bit kp=0;
	P3=0xf0;
	scan1=P3&0xf0;
	if(scan1!=0xf0)
	{
		delayms(10);
		scan1=P3&0xf0;
		if((scan1!=0xf0)&&(kp==0))
		{
			kp=1;
			P3=0x0f;
			scan2=P3%0x0f;
			temp=scan1|scan2;
			return temp;
		}
	}
	else kp=0;
	return 0xff;
}
uchar key_pro()
{
	switch(key_scan())
	{
		case 0x7e:return '/';break;//°´¼ü0
		case 0x7d:return 'x';break;//°´¼ü1
		case 0x7b:return '-';break;//°´¼ü2
		case 0x77:return '+';break;//°´¼ü3
		case 0xbe:return '=';break;//°´¼ü4
		case 0xbd:return '3';break;//°´¼ü5
		case 0xbb:return '6';break;//°´¼ü6
		case 0xb7:return '9';break;//°´¼ü7
		case 0xde:return '.';break;//°´¼ü8
		case 0xdd:return '2';break;//°´¼ü9
		case 0xdb:return '5';break;//°´¼üA
		case 0xd7:return '8';break;//°´¼üB
		case 0xee:return '0';break;//°´¼üC
		case 0xed:return '1';break;//°´¼üD
		case 0xeb:return '4';break;//°´¼üE
		case 0xe7:return '7';break;//°´¼üF
		default:return 0xff;break;
	}
}
void main(void)
{
	uchar num,i,sign;
	uchar temp[16];
	bit firstflag;
	float a=0,b=0;
	uchar s;
	LCD_init();
	delayms(10);
	w_com(0x01);
	delayms(200);
	w_com(0x01);
	while(1)
	{
		num=key_pro();
		if(num!=0xff)         //Èç¹ûÉ¨ÃèÊ±ÓÐÐ§ÖµÔòÏÂÒ»²½
		{
			if(i==0)            //ÊäÈëµÚÒ»¸ö×Ö·ûµÄÊ±ºò£¬ÐèÒª°ÑºóÃæÇå¿Õ
				w_com(0x01);
			if(('+'==num)||(i==16)||('-'==num)||('x'==num)||('/'==num)||('='==num))
			{
				i=0;              //¼ÆËãÆ÷¸´Î»
				if(firstflag==0)//flagµÈÓÚ0£¬ÔòËµÃ÷Ö®Ç°Ã»ÊéÈç¹ûÊý×Ö£¬ÏÖÔÚÊäÈëÒ»¸ö±»¼ÓÊý
        {
					sscanf(temp,"%f",&a);//ÊäÈë±»¼ÓÊý
					firstflag=1;
				}
				else
					sscanf(temp,"%f",&b);//Èç¹ûflagµÈÓÚ1£¬ÔòÖ®Ç°ÊäÈëÁËÒ»¸ö±»¼Ó»ò¼õ¡¢³Ë¡¢³ýÊý×
				for(s=0;s<16;s++) //»º³åÇøÇåÀí
				{
					temp[s]=0;
				}
				w_Char(0,1,num);  //·ûºÅÔÚµÚ¶þÐÐ
				if(num!='=')
					sign=num;       //Èç¹û²»ÊÇµÈºÅ£¬¼ÇÏÂ±êÖ¾Î»
				else
				{
					firstflag=0;    //¼ì²âµ½ÊäÈë=ºÅ£¬ÅÐ¶ÏÉÏ´Î¶ÁÈëµÄ·ûºÅ
					switch(sign)
					{
						case '+':a=a+b;break;
						case '-':a=a-b;break;
						case 'x':a=a*b;break;
						case '/':a=a/b;break;
						default:break;
					}
					sprintf(temp,"%g",a);  //Êä³ö¸¡µãÐÍ
					w_str(1,1,temp);       //ÏÔÊ¾µ½Òº¾§ÆÁ
					sign=0;a=b=0;          //Ö®ºóÊý¾ÝÇåÁã
					for(s=0;s<16;s++)
					temp[s]=0;
				}
			}
			else if(i<16)
			{
				if((1==i)&&(temp[0]=='0'))  //Èç¹ûµÚÒ»¸ö×Ö·ûÎª0£¬Ôò¶ÔÏÂÒ»¸ö×Ö·û½øÐÐÅÐ¶Ï
				{
					if(num=='.')       //Èç¹ûÐ¡Êýµã£¬Ôò¹â±êÎ»ÖÃ¼Ó1
					{
						temp[1]='.';
						w_Char(1,0,num); //Êä³öÊý¾Ý
						i++;
					}
					else
					{
						temp[0]=num;     //Èç¹ûÊÇÊý×Ö1-9£¬ÔòËµÃ÷0Ã»ÓÃ£¬ÔòÌæ»»0ËùÔÚµÄµÚÒ»Î»
						w_Char(0,0,num); //Êä³öÊý¾Ý
					}
				}
				else
				{
					temp[i]=num;
					w_Char(i,0,num);   //Êä³öÊý¾Ý
					i++;               //ÊäÈëÊýÖµÀÛ¼Ó
				}
			}
		}
	}
}
