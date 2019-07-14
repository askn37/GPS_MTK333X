/***************
 *
 * GPS_MTK333X_I2C - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_I2C()					// use Wire (TWI)
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __GPS_MTK333X_I2C_H
#define __GPS_MTK333X_I2C_H

#include <Wire.h>
#include "GPS_MTK333X.h"

// I2C values
#define MT333X_ADDR         0x10
#define I2C_SPEED_STANDARD  100000
#define I2C_SPEED_DOUBLE    200000
#define I2C_SPEED_FAST      400000

//
// I2C interface
//
class GPS_MTK333X_I2C : public GPS_MTK333X {
private :
	long _speed = 0;
	uint8_t _i2caddr = 0;
	uint8_t _int = 0xFF;
	char _buff[33];

public :
	GPS_MTK333X_I2C (void) {};
	GPS_MTK333X_I2C (uint8_t intPin) { _int = intPin; }
	using GPS_MTK333X::GPS_MTK333X;
	using super = GPS_MTK333X;
    void setClock (long speed) { _speed = speed; Wire.setClock(_speed); }

	//
	// GPS_MTK333X I2C interface class
	//
    bool begin (long i2cSpeed = I2C_SPEED_FAST, uint8_t i2caddr = MT333X_ADDR) {
		if (!_speed) Wire.begin();
		_i2caddr = i2caddr;
		_speed = i2cSpeed;
		return sendMTKcommand(0, F(""));
	}

	bool check (void) {
		if (_int != 0xFF && digitalRead(_int)) return false;
		Wire.setClock(_speed);
		if (Wire.requestFrom(_i2caddr, 32U)) {
			uint8_t j = 0;
			for (uint32_t i = 0; i < 32U; i++) {
				if (Wire.available()) {
					char c = 0x7F & Wire.read();
					if (c > ' ') _buff[j++] = c;
				}
			}
			if (j) _buff[j] = 0;
			for (uint32_t i = 0; i < j; i++) {
				encode(_buff[i]);
			}
		}
		return isLineFeed();
	}

	bool sendMTKcommand (uint16_t packetType, String dataField = "") {
		while (!check());
		String command = createMTKpacket(F("MTK"), packetType, dataField);
		int len = command.length();
		Wire.setClock(_speed);
		Wire.beginTransmission(_i2caddr);
		for (int i = 0; i < len; i++) {
			if (i != 0 && (i & 31) == 0) {
				Wire.endTransmission();
				delayMicroseconds(1000);
				Wire.beginTransmission(_i2caddr);
			}
			Wire.write(command[i]);
		}
		Wire.endTransmission();
		// return resultMTKcommand(check, packetType);
		setMessage(-1);
		uint32_t ms = millis();
		while ((millis() - ms) < 1000U) {
			if (check() && getMessage() == packetType) return (getResult() == 3U);
		}
		return false;
	}

	bool sendUBXcommand (uint16_t packetType, String dataField = "") {
		while (!check());
		String command = createMTKpacket(F("UBX"), packetType, dataField);
		int len = command.length();
		Wire.setClock(_speed);
		Wire.beginTransmission(_i2caddr);
		for (int i = 0; i < len; i++) {
			if (i != 0 && (i & 31) == 0) {
				Wire.endTransmission();
				delayMicroseconds(1000);
				Wire.beginTransmission(_i2caddr);
			}
			Wire.write(command[i]);
		}
		Wire.endTransmission();
		return false;
	}

};

#endif