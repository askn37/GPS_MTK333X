/***************
 *
 * GPS_MTK333X_Serial2 - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_Serial2()
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __GPS_MTK333X_SERIAL2_H
#define __GPS_MTK333X_SERIAL2_H

#include "GPS_MTK333X.h"

//
// UART interface
//
class GPS_MTK333X_Serial2 : public GPS_MTK333X {
public :
    using GPS_MTK333X::GPS_MTK333X;
    using super = GPS_MTK333X;

    //
    // GPS_MTK333X UART interface class
    //
    inline int available (void) { return Serial2.available(); }
    inline size_t write (const uint8_t c) { return Serial2.write(c); }
    inline int peek (void) { return Serial2.peek(); }
    inline int read (void) { return Serial2.read(); }
    inline void flush (void) { Serial2.flush(); }
    bool begin (long baud) {
        Serial2.begin(baud);
		return sendMTKcommand(0, F(""));
    }

    bool check (void) {
        while (Serial2.available()) {
            encode(Serial2.read());
        }
        return isLineFeed();
    }

    bool sendMTKcommand (uint16_t packetType, String dataField) {
        while (!check());
        Serial2.print(createMTKpacket(F("MTK"), packetType, dataField));
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
        Serial2.print(createMTKpacket(F("UBX"), packetType, dataField));
        return false;
    }

};

#endif

// end of header
