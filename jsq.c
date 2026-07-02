#include <reg52.h>
#include <stdio.h>

typedef unsigned char uint8;
typedef unsigned int uint16;

#define KEY_NONE 0xff
#define INPUT_BUFFER_SIZE 16
#define DELAY_1MS_LOOP_COUNT 123

#define LCD_LINE_1_ADDR 0x80
#define LCD_LINE_2_ADDR 0xc0
#define LCD_CLEAR_COMMAND 0x01
#define LCD_FUNCTION_SET 0x38
#define LCD_ENTRY_MODE_SET 0x06
#define LCD_DISPLAY_ON 0x0c

#define KEYPAD_HIGH_MASK 0xf0
#define KEYPAD_LOW_MASK 0x0f

sbit LCD_RS = P2^4;
sbit LCD_RW = P2^5;
sbit LCD_ENABLE = P2^6;

/**
 * 延迟函数
 * 11.0592MHz晶振下，延迟约1ms
 */
static void delay_ms(uint16 ms)
{
	uint8 loop_index;

	while (ms--)
	{
		for (loop_index = 0; loop_index < DELAY_1MS_LOOP_COUNT; loop_index++)
		{
		}
	}
}

/**
 * 写命令函数
 */
static void lcd_write_command(uint8 command)
{
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_ENABLE = 1;
	P0 = command;
	LCD_ENABLE = 0;
	delay_ms(1);
}

/**
 * 写入数据
 */
static void lcd_write_data(uint8 data_byte)
{
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_ENABLE = 1;
	P0 = data_byte;
	LCD_ENABLE = 0;
	delay_ms(1);
}

/**
 * 写入字符
 */
static void lcd_write_char(uint8 column, uint8 row, uint8 data_byte)
{
	if (row == 0)
	{
		lcd_write_command(LCD_LINE_1_ADDR + column);
	}
	else
	{
		lcd_write_command(LCD_LINE_2_ADDR + column);
	}
	lcd_write_data(data_byte);
}

/**
 * 写入字符串
 */
static void lcd_write_string(uint8 column, uint8 row, char *text)
{
	while (*text)
	{
		lcd_write_char(column, row, *text);
		text++;
		column++;
	}
}

/**
 * 液晶屏初始化
 */
static void lcd_init(void)
{
	delay_ms(10);
	lcd_write_command(LCD_FUNCTION_SET);
	delay_ms(10);
	lcd_write_command(LCD_ENTRY_MODE_SET);
	delay_ms(10);
	lcd_write_command(LCD_DISPLAY_ON);
	delay_ms(10);
	lcd_write_command(LCD_CLEAR_COMMAND);
	delay_ms(10);
	lcd_write_command(LCD_FUNCTION_SET);
	delay_ms(10);
}

/**
 * 4x4矩阵按键扫描
 * 使用按键扫描法
 */
static uint8 keypad_scan(void)
{
	uint8 row_bits;
	uint8 column_bits;
	uint8 key_code;
	static bit is_key_latched = 0;

	P3 = KEYPAD_HIGH_MASK;
	row_bits = P3 & KEYPAD_HIGH_MASK;
	if (row_bits != KEYPAD_HIGH_MASK)
	{
		delay_ms(10);
		row_bits = P3 & KEYPAD_HIGH_MASK;
		if ((row_bits != KEYPAD_HIGH_MASK) && (is_key_latched == 0))
		{
			is_key_latched = 1;
			P3 = KEYPAD_LOW_MASK;
			column_bits = P3 & KEYPAD_LOW_MASK;
			key_code = row_bits | column_bits;
			return key_code;
		}
	}
	else
	{
		is_key_latched = 0;
	}
	return KEY_NONE;
}

/**
 * 定义各按键含义
 */
static uint8 keypad_get_key(void)
{
	switch (keypad_scan())
	{
		case 0x7e: return '/';	//按键0
		case 0x7d: return 'x';	//按键1
		case 0x7b: return '-';	//按键2
		case 0x77: return '+';	//按键3
		case 0xbe: return '=';	//按键4
		case 0xbd: return '3';	//按键5
		case 0xbb: return '6';	//按键6
		case 0xb7: return '9';	//按键7
		case 0xde: return '.';	//按键8
		case 0xdd: return '2';	//按键9
		case 0xdb: return '5';	//按键A
		case 0xd7: return '8';	//按键B
		case 0xee: return '0';	//按键C
		case 0xed: return '1';	//按键D
		case 0xeb: return '4';	//按键E
		case 0xe7: return '7';	//按键F
		default: return KEY_NONE;
	}
}

/**
 * 清空输入缓冲区
 */
static void clear_input_buffer(char *buffer)
{
	uint8 index;
	for (index = 0; index < INPUT_BUFFER_SIZE; index++)
	{
		buffer[index] = 0;
	}
}

/**
 * 判断是否为运算符按键
 */
static uint8 is_operator_key(uint8 key)
{
	return (key == '+') || (key == '-') || (key == 'x') || (key == '/') || (key == '=');
}

/**
 * 将输入字符串转换为浮点数
 */
static float parse_input_number(char *text)
{
	float value = 0;
	float decimal_scale = 0.1;
	bit is_fraction_part = 0;

	while (*text)
	{
		if (*text == '.')
		{
			if (is_fraction_part)
			{
				break;
			}
			is_fraction_part = 1;
		}
		else if ((*text >= '0') && (*text <= '9'))
		{
			if (is_fraction_part)
			{
				value += (*text - '0') * decimal_scale;
				decimal_scale *= 0.1;
			}
			else
			{
				value = value * 10 + (*text - '0');
			}
		}
		else
		{
			break;
		}
		text++;
	}

	return value;
}

/**
 * 主函数
 */
int main(void)
{
	uint8 pressed_key = KEY_NONE;
	uint8 input_index = 0;
	uint8 operator_key = 0;
	char input_buffer[INPUT_BUFFER_SIZE] = {0};
	bit has_first_operand = 0;
	bit has_calculation_error = 0;
	float first_operand = 0;
	float second_operand = 0;

	lcd_init();
	delay_ms(10);
	lcd_write_command(LCD_CLEAR_COMMAND);
	delay_ms(200);
	lcd_write_command(LCD_CLEAR_COMMAND);
	while (1)
	{
		pressed_key = keypad_get_key();
		if (pressed_key != KEY_NONE)         //如果扫描有效值则进入下一步
		{
			if (input_index == 0)            //输入第一个字符的时，需要把后面清空
			{
				lcd_write_command(LCD_CLEAR_COMMAND);
			}
			if (is_operator_key(pressed_key) || (input_index == INPUT_BUFFER_SIZE))
			{
				input_index = 0;              //计算器复位
				if (has_first_operand == 0)//还没有第一个操作数时，当前输入作为第一个操作数
				{
					first_operand = parse_input_number(input_buffer);//输入被加数
					has_first_operand = 1;
				}
				else
				{
					second_operand = parse_input_number(input_buffer);//已有第一个操作数时，当前输入作为第二个操作数
				}
				clear_input_buffer(input_buffer);
				lcd_write_char(0, 1, pressed_key);  //符号在第二行
				if (pressed_key != '=')
				{
					operator_key = pressed_key;       //如果输入的不是等号，记下标志位
				}
				else
				{
					has_first_operand = 0;    //检测到输入'='号，判断上次读入的符号
					has_calculation_error = 0;
					switch (operator_key)
					{
						case '+':
							first_operand = first_operand + second_operand;
							break;
						case '-':
							first_operand = first_operand - second_operand;
							break;
						case 'x':
							first_operand = first_operand * second_operand;
							break;
						case '/':
							if (second_operand == 0)
							{
								has_calculation_error = 1;
							}
							else
							{
								first_operand = first_operand / second_operand;
							}
							break;
						default:
							break;
					}
					if (has_calculation_error)
					{
						lcd_write_char(1, 1, 'E');
						lcd_write_char(2, 1, 'r');
						lcd_write_char(3, 1, 'r');
					}
					else
					{
						sprintf(input_buffer, "%.6g", first_operand);  //输出浮点型
						lcd_write_string(1, 1, input_buffer);       //显示到液晶屏
					}
					operator_key = 0;    //之后数据清零
					first_operand = second_operand = 0;   //之后数据清零            
					clear_input_buffer(input_buffer);
				}
			}
			else if (input_index < INPUT_BUFFER_SIZE)
			{
				if ((1 == input_index) && (input_buffer[0] == '0'))  //如果第一个字符为0，则对下一个字符进行判断
				{
					if (pressed_key == '.')       //如果是小数点，则光标位置加1
					{
						input_buffer[1] = '.';
						lcd_write_char(1, 0, pressed_key);   //输出数据
						input_index++;
					}
					else
					{
						input_buffer[0] = pressed_key;      //如果是数字1-9，则说明0没用，则替换0所在的第一位
						lcd_write_char(0, 0, pressed_key);  //输出数据
					}
				}
				else
				{
					input_buffer[input_index] = pressed_key;
					lcd_write_char(input_index, 0, pressed_key);   //输出数据
					input_index++;               //输入数值累加
				}
			}
		}
	}
}
