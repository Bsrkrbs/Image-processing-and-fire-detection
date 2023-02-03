#include "LiquidCrystal.h"

using namespace std;

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	_rs_pin = rs;
	_enable_pin = enable;
	_numlines = 2;

	_data_pins[0] = d0;
	_data_pins[1] = d1;
	_data_pins[2] = d2;
	_data_pins[3] = d3; 

	_displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;

	begin(16, 2);  
	
	createChar(0,tr_s);
	createChar(1,tr_u);
	createChar(2,tr_i);
	createChar(3,tr_c);
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
	setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  
	
	//Pinlerin yönü ayarlanıyor
	bcm2835_gpio_fsel(_rs_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(_enable_pin, BCM2835_GPIO_FSEL_OUTP);
	for (int i=0; i<4; i++)
	{
		bcm2835_gpio_fsel(_data_pins[i], BCM2835_GPIO_FSEL_OUTP);
	} 

	bcm2835_delayMicroseconds(50000);
	//Komut yazmak için RS pini low yapılıyor
	bcm2835_gpio_clr(_rs_pin);
	bcm2835_gpio_clr(_enable_pin);
  
	//ilk reset
    write4bits(0x03);
    bcm2835_delayMicroseconds(4500);

    write4bits(0x03);
    bcm2835_delayMicroseconds(4500);

    write4bits(0x03); 
    bcm2835_delayMicroseconds(150);

    write4bits(0x02); 
	
	command(LCD_FUNCTIONSET | _displayfunction);  

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
	display();

	clear();

	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::command(uint8_t value) 
{
	send(value, LOW);
}

void LiquidCrystal::setRowOffsets(int row0, int row1, int row2, int row3)
{
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}

void LiquidCrystal::clear()
{
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	bcm2835_delayMicroseconds(2000);
}

void LiquidCrystal::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  bcm2835_delayMicroseconds(2000);
}

void LiquidCrystal::noDisplay()
{
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::display() 
{
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::noBlink() 
{
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::blink() 
{
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::noCursor() 
{
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::cursor() 
{
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::scrollDisplayLeft(void) 
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal::scrollDisplayRight(void) 
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LiquidCrystal::leftToRight(void) 
{
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::rightToLeft(void) 
{
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::autoscroll(void) 
{
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::noAutoscroll(void) 
{
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::createChar(uint8_t location, uint8_t charmap[]) 
{
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row)
{
	const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
	if ( row >= max_lines ) {
		row = max_lines - 1;    // we count rows starting w/0
	}
	if ( row >= _numlines ) {
		row = _numlines - 1;    // we count rows starting w/0
	}

	command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

size_t LiquidCrystal::write(uint8_t value) 
{
	send(value, HIGH);
	return 1;
}

void LiquidCrystal::sendText(std::string value)
{
	wstring_convert<codecvt_utf8<char32_t>, char32_t> cn;
	auto ws = cn.from_bytes(value);
	
	for(int i = 0; i < ws.size(); i++) 
	{
		//printf("%d\n",ws[i]);
		switch(ws[i]) 
		{
			case 351 : // ş
				write(0);
				break;
			case 252 : // ü
				write(1);
				break;
			case 305 : // ı
				write(2);
				break;
			case 231 : // ç
				write(3);
				break;
			default :
				write(ws[i]);
		}
	}
}

//private functions
void LiquidCrystal::send(uint8_t value, uint8_t mode) 
{
	bcm2835_gpio_write(_rs_pin, mode);
	write4bits(value>>4);
	write4bits(value);
}

void LiquidCrystal::write4bits(uint8_t value) 
{
	for (int i = 0; i < 4; i++) 
	{
		bcm2835_gpio_write(_data_pins[i],(value >> i) & 0x01);
	}

	pulseEnable();
}

void LiquidCrystal::pulseEnable(void) 
{
	bcm2835_gpio_clr(_enable_pin);
	bcm2835_delayMicroseconds(1);   
	bcm2835_gpio_set(_enable_pin);
	bcm2835_delayMicroseconds(1);    // enable pulse must be >450ns
	bcm2835_gpio_clr(_enable_pin);
	bcm2835_delayMicroseconds(100);   // commands need > 37us to settle
}