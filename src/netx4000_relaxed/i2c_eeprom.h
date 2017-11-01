#include "boot_common.h"
#include "i2c_interface.h"


#ifndef __I2C_EEPROM_H__
#define __I2C_EEPROM_H__


typedef struct STRUCT_I2C_OPTIONS
{
	unsigned long  ulNewOffset;
	unsigned char  ucEepromAddress;
} I2C_OPTIONS_T;


BOOTING_T boot_i2c_eeprom(const I2C_FUNCTIONS_T *ptI2cFn, unsigned long ulMagic);


#endif  /* __I2C_EEPROM_H__ */

