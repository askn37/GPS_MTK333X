/***************
 *
 * GPS_MTK333X_MultiUART - GPS MTK333X Interface for Arduino
 *
 * call constructor:
 *
 *		GPS_MTK333X_MultiUART(RX_PIN, TX_PIN)	// use MultiUART
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

// #include <MultiUART.h>
#if defined(__MULTIUART_H)
#ifndef __GPS_MTK333X_MULTIUART_H
#define __GPS_MTK333X_MULTIUART_H

#include "GPS_MTK333X.h"

//
// UART interface
//
class GPS_MTK333X_MultiUART : public GPS_MTK333X {
private :
    MultiUART* _UART;
public :
    using GPS_MTK333X::GPS_MTK333X;
    using super = GPS_MTK333X;

    //
    // GPS_MTK333X UART interface class
    //
    inline int available (void) { return _UART->available(); }
    inline size_t write (const uint8_t c) { return _UART->write(c); }
    inline int peek (void) { return _UART->peek(); }
    inline int read (void) { return _UART->read(); }
    inline void flush (void) { _UART->flush(); }
    bool begin (long baud) {
        _UART->begin(baud);
		return sendMTKcommand(0, F(""));
    }

    GPS_MTK333X_MultiUART (uint8_t receivePin, uint8_t transmitPin) {
        _UART = new MultiUART(receivePin, transmitPin);
    }

    bool check (void) {
        while (_UART->available()) {
            encode(_UART->read());
        }
        return isLineFeed();
    }

    bool sendMTKcommand (uint16_t packetType, String dataField) {
        while (!check());
        _UART->print(createMTKpacket(F("MTK"), packetType, dataField));
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
        _UART->print(createMTKpacket(F("UBX"), packetType, dataField));
        return false;
    }

};

#endif
#endif

// end of header
