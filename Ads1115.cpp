#include "Ads1115.h"

Ads1115::Ads1115(uint8_t i2cAddress) 
{
	m_i2cAddress = i2cAddress;
	m_conversionDelay = CONVERSIONDELAY;
	m_gain = GAIN_ONE; /* +/- 4.096V range */
	
	bcm2835_i2c_setSlaveAddress(m_i2cAddress);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
}

void Ads1115::setGain(adsGain_t gain) { m_gain = gain; }

adsGain_t Ads1115::getGain() { return m_gain; }

uint16_t Ads1115::readRegister(uint8_t reg) 
{
	//char[] r = {reg};
	char dataConversion[2] = {0};
	bcm2835_i2c_write((char*)&reg,1);
	bcm2835_i2c_read(dataConversion,2);
	return ((dataConversion[0] << 8) | dataConversion[1]);
}

void Ads1115::writeRegister(uint8_t reg, uint16_t value) 
{
	char r[] = {reg, (char)(value >> 8), (char)(value & 0xFF)};
	bcm2835_i2c_write(r,3);
}

uint16_t Ads1115::readADC_SingleEnded(uint8_t channel) 
{
	if (channel > 3) 
	{
		return 0;
	}

	uint16_t config =
		REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
		REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
		REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
		REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
		REG_CONFIG_DR_128SPS |   // 8 samples per second 
		REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

	// Set PGA/voltage range
	config |= m_gain;

	// Set single-ended input channel
	switch (channel) 
	{
		case (0):
			config |= REG_CONFIG_MUX_SINGLE_0;
			break;
		case (1):
			config |= REG_CONFIG_MUX_SINGLE_1;
			break;
		case (2):
			config |= REG_CONFIG_MUX_SINGLE_2;
			break;
		case (3):
			config |= REG_CONFIG_MUX_SINGLE_3;
			break;
	}

	// Set 'start single-conversion' bit
	config |= REG_CONFIG_OS_SINGLE;

	// Write config register to the ADC
	writeRegister(REG_POINTER_CONFIG, config);

	// Wait for the conversion to complete
	bcm2835_delay(CONVERSIONDELAY);

	return readRegister(REG_POINTER_CONVERT);
}
