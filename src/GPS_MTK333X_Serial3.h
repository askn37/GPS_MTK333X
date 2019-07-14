/***************
 *
 * GPS_MTK333X_Serial3 - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_Serial3()
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __GPS_MTK333X_SERIAL3_H
#define __GPS_MTK333X_SERIAL3_H

#include "GPS_MTK333X.h"

//
// UART interface
//
class GPS_MTK333X_Serial3 : public GPS_MTK333X {
public :
    using GPS_MTK333X::GPS_MTK333X;
    using super = GPS_MTK333X;

    //
    // GPS_MTK333X UART interface class
    //
    inline int available (void) { return Serial3.available(); }
    inline size_t write (const uint8_t c) { return Serial3.write(c); }
    inline int peek (void) { return Serial3.peek(); }
    inline int read (void) { return Serial3.read(); }
    inline void flush (void) { Serial3.flush(); }
    bool begin (long baud) {
        Serial3.begin(baud);
		return sendMTKcommand(0, F(""));
    }

    bool check (void) {
        while (Serial3.available()) {
            encode(Serial3.read());
        }
        return isLineFeed();
    }

    bool sendMTKcommand (uint16_t packetType, String dataField) {
        while (!check());
        Serial3.print(createMTKpacket(F("MTK"), packetType, dataField));
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
        Serial3.print(createMTKpacket(F("UBX"), packetType, dataField));
        return false;
    }

};

#endif

// end of header
