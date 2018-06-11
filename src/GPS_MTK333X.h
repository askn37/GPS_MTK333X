/***************
 *
 * GPS_MTK333X - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_UART(Serial1)			// use HardwareSerial
 *		GPS_MTK333X_UART(RX_PIN, TX_PIN)	// use MultiUART
 *		GPS_MTK333X_I2C()					// use Wire (TWI)
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __GPS_MTK333X_H
#define __GPS_MTK333X_H

#include <Arduino.h>
#include <Wire.h>
#include <bcdtime.h>

#ifndef SoftwareSerial_h
#define SUPER_SERIAL	MultiUART
#include <MultiUART.h>
#else
#define SUPER_SERIAL	SoftwareSerial
#include <SoftwareSerial.h>
#endif

// I2C values
#define MT333X_ADDR         0x10
#define I2C_SPEED_STANDARD  100000
#define I2C_SPEED_DOUBLE    200000
#define I2C_SPEED_FAST      400000

// NMEA sentence
#define _NMEA_GGA           0x01U
#define _NMEA_RMC           0x02U
#define _NMEA_PMTK001       0x0FU

struct GPSInfo_t {
	bcddate_t date;
	bcdtime_t time;
	int32_t latitude;
	int32_t longitude;
	int32_t altitude;
	uint32_t speed;
	uint16_t course;
	uint16_t satellites;
	uint16_t dop;
	uint16_t millisecond;
};

typedef union {
	GPSInfo_t value;
	uint32_t colums[8];
} GPSInfo_colum_t;

//
// application core class
//
class GPS_MTK333X {
private :
	GPSInfo_t _GPSInfo;

	uint32_t t_time;
	uint32_t t_date;
	uint32_t t_millis;
	uint32_t t_speed;

	int32_t t_lat;
	int32_t t_lng;
	int32_t t_alt;
	uint16_t t_course;
	uint16_t t_dop;
	uint8_t t_sate;

	uint16_t _message;
	uint8_t _nmeaBuff[12];
	uint8_t _colum;
	uint8_t _parity;
	uint8_t _sentence;
	uint8_t _offset:4;

	uint8_t _result:3;
	uint8_t _isRMC:2;
	uint8_t _isGGA:1;
	uint8_t _isParity:1;
	uint8_t _isTime:1;
	uint8_t _isDate:1;
	uint8_t _isLatitude:1;
	uint8_t _isLongitude:1;
	uint8_t _isAltitude:1;
	uint8_t _isCourse:1;
	uint8_t _isSpeed:1;
	uint8_t _isLineFeed:1;
	uint8_t _isTimeUpdate:1;
	uint8_t _isLocUpdate:1;
	uint8_t _isStatUpdate:1;

	void parser (void);
	void parserCommit (void);
	uint8_t fromHex (const uint8_t);
	int32_t fromDegrees (int16_t);

public :
	GPS_MTK333X (void);
	// ~GPS_MTK333X (void);

	bool encode (const uint8_t);
	inline GPSInfo_t value (void) {
		statusReset();
		return _GPSInfo;
	};
	inline void statusReset (void) {
		_isLocUpdate = _isTimeUpdate = _isStatUpdate = false;
	}

	String createMTKpacket (uint16_t, String = "");
	String calcCRCforMTK (String);
	// bool resultMTKcommand (bool (*)(void), uint16_t);

	inline bool isLineFeed (void) { return _isLineFeed; }
	inline bool isLocationUpdate (void) { return _isLocUpdate; }
	inline bool isTimeUpdate (void) { return _isTimeUpdate; }
	inline bool isStatusUpdate (void) { return _isStatUpdate; }

	inline void setMessage (uint16_t message) { _message = message; }
	inline uint16_t getMessage (void) { return _message; }
	inline uint8_t getResult (void) { return _result; }

	inline bcddate_t date (void) { return _GPSInfo.date; }
	inline bcdtime_t time (void) { return _GPSInfo.time; }
	inline time_t epoch (void) { return bcdToEpoch(_GPSInfo.date, _GPSInfo.time); }
	inline uint16_t millisecond (void) { return _GPSInfo.millisecond; }
	inline int32_t latitude (void) { return _GPSInfo.latitude; }
	inline int32_t longitude (void) { return _GPSInfo.longitude; }
	inline int32_t altitude (void) { return _GPSInfo.altitude; }
	inline uint32_t speed (void) { return _GPSInfo.speed; }
	inline uint16_t course (void) { return _GPSInfo.course; }
	inline uint16_t dop (void) { return _GPSInfo.dop; }
	inline uint16_t satellites (void) { return _GPSInfo.satellites; }
};

//
// UART interface
//
class GPS_MTK333X_UART : public SUPER_SERIAL, public GPS_MTK333X {
private :
	long _speed = 0;
public :
	using SUPER_SERIAL::SUPER_SERIAL;
	using super = SUPER_SERIAL;
    bool begin (long);
	bool check (void);
	boolean sendMTKcommand (uint16_t, String = "");
};

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
    bool begin (long = I2C_SPEED_FAST, uint8_t = MT333X_ADDR);
	bool check (void);
	bool sendMTKcommand (uint16_t, String = "");
};

#endif

// end of ehader
