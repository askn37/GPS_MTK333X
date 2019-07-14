/***************
 *
 * GPS_MTK333X_Serial - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_Serial()
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __GPS_MTK333X_SERIAL_H
#define __GPS_MTK333X_SERIAL_H

#include "GPS_MTK333X.h"

//
// UART interface
//
class GPS_MTK333X_Serial : public GPS_MTK333X {
public :
    using GPS_MTK333X::GPS_MTK333X;
    using super = GPS_MTK333X;

    //
    // GPS_MTK333X UART interface class
    //
    inline int available (void) { return Serial.available(); }
    inline size_t write (const uint8_t c) { return Serial.write(c); }
    inline int peek (void) { return Serial.peek(); }
    inline int read (void) { return Serial.read(); }
    inline void flush (void) { Serial.flush(); }
    bool begin (long baud) {
        Serial.begin(baud);
		return sendMTKcommand(0, F(""));
    }

    bool check (void) {
        while (Serial.available()) {
            encode(Serial.read());
        }
        return isLineFeed();
    }

    bool sendMTKcommand (uint16_t packetType, String dataField) {
        while (!check());
        Serial.print(createMTKpacket(F("MTK"), packetType, dataField));
        // return resultMTKcommand(check, packetType);
        setMessage(-1);
        uint32_t ms = millis();
        while ((millis() - ms) < 1000U) {
            if (check() && getMessage() == packetType) return (getResult() == 3U);
        }
        return false;
    }

    bool sendUBXcommand (uint16_t packetType, String dataField) {
        while (!check());
        Serial.print(createMTKpacket(F("UBX"), packetType, dataField));
        return false;
    }

};

#endif

// end of header
