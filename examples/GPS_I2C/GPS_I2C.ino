/***************
 *
 * GPS_I2C - GPS MTK333X Interface I2C sample for Arduino
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#include <Arduino.h>
#include "GPS_MTK333X.h"

#define CONSOLE_BAUD	9600

#define LED_BUILTIN     13
#define GPS_PPS         A0      // I High GPS purse per seconds
#define GPS_INT         A2      // I Low GPS I2C Buffer active
#define GPS_WAKE        A3      // O Negative Sleep GPS

// GPS_MTK333X_I2C GPS;
GPS_MTK333X_I2C GPS(GPS_INT);   // use sensing interrupt pin

void setup (void) {
    pinMode(GPS_INT, INPUT_PULLUP);

    Serial.begin(CONSOLE_BAUD);
    Serial.println(F("Startup"));

    while (!GPS.begin()) {
        Serial.println(F("GPS notready"));
        delay(1000);
    }

    // GPS.sendMTKcommand(220, F(",1000"));			// 220 PMTK_API_SET_FIX_CTL (MTK3339)
    GPS.sendMTKcommand(300, F(",1000,0,0,0,0"));	// 300 PMTK_API_SET_FIX_CTL
    GPS.sendMTKcommand(225, F(",0"));				// 225 PMTK_SET_PERIODIC_MODE
    // GPS.sendMTKcommand(353, F(",1,0,0,0,0"));
    GPS.sendMTKcommand(351, F(",1"));				// 351 PMTK_API_SET_SUPPORT_QZSS_NMEA
    GPS.sendMTKcommand(314, F(",0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
}

void loop (void) {
    if (GPS.check() && GPS.isTimeUpdate()) {
        bool f = GPS.isStatusUpdate();
        GPSInfo_t gpsInfo = GPS.value();
        Serial.print(gpsInfo.date, HEX);
        Serial.write(' ');
        Serial.print(gpsInfo.time, HEX);
        Serial.write(' ');
        if (f) {
            Serial.print(gpsInfo.satellites, DEC);
            Serial.write(' ');
            Serial.print(gpsInfo.dop * 0.01);
            Serial.write(' ');
            Serial.print(gpsInfo.latitude / 600000.0, 6);
            Serial.write(' ');
            Serial.print(gpsInfo.longitude / 600000.0, 6);
            Serial.write(' ');
            Serial.print(gpsInfo.altitude * 0.01);
            Serial.write(' ');
            Serial.print(gpsInfo.speed * 0.01852);
            Serial.write(' ');
            Serial.print(gpsInfo.course * 0.01);
            Serial.write(' ');
        }
        Serial.println();
    }
}

// end of code
