/***************
 *
 * GPS_MTK333X - GPS MTK333X Interface for Arduino
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#include <Arduino.h>
#include <Wire.h>
#include "GPS_MTK333X.h"

//
// GPS_MTK333X application class
//
GPS_MTK333X::GPS_MTK333X (void) {
    _offset = _parity = _sentence = _isRMC = 0;
    _isParity =
    _isTime =
    _isDate =
    _isLatitude =
    _isLongitude =
    _isAltitude =
    _isCourse =
    _isSpeed =
    _isLineFeed	=
    _isTimeUpdate =
    _isLocUpdate =
    _isStatUpdate =
    _isGGA = false;
    _GPSInfo = {0,0,0,0,0,0,0,0,0,0};
}

bool GPS_MTK333X::encode (const uint8_t c) {
    // Serial.write(c);
    switch (c) {
        case '$' : {
            _parity = _offset = _colum = _sentence = _isRMC = 0U;
            _isParity = _isLineFeed = _isGGA = false;
            break;
        }
        case ',' : _parity ^= (byte) c;
        case '*' : {
            if (_offset < sizeof(_nmeaBuff)) {
                _nmeaBuff[_offset] = 0;
                if (_offset) parser();
            }
            _offset = 0;
            _colum++;
            if (c == '*') _isParity = true;
            break;
        }
        case '\r' :
        case '\n' : break;
        default : {
            if (_offset < sizeof(_nmeaBuff) -1) {
                _nmeaBuff[_offset++] = c;
            }
            if (_isParity) {
                _isLineFeed = true;
                if (_offset == 2) parserCommit();
            }
            else {
                _parity ^= (uint8_t) c;
            }
        }
    }
    return _isLineFeed;
};

void GPS_MTK333X::parserCommit (void) {
    if (_parity == ((fromHex(_nmeaBuff[0]) << 4) | fromHex(_nmeaBuff[1]))) {
        if (_isRMC) {
            if (_isDate) {
                _GPSInfo.date = t_date;
                _isTimeUpdate = true;
            }
            if (_isTime) {
                _GPSInfo.time = t_time;
                _GPSInfo.millisecond = t_millis;
            }
            if (_isRMC > 1) {
                if (_isLatitude)  _GPSInfo.latitude  = t_lat;
                if (_isLongitude) _GPSInfo.longitude = t_lng;
                if (_isSpeed)     _GPSInfo.speed     = t_speed;
                if (_isCourse)    _GPSInfo.course    = t_course;
                _isStatUpdate = true;
            }
            _isRMC = 0;
        }
        if (_isGGA) {
            if (_isLatitude)  _GPSInfo.latitude  = t_lat;
            if (_isLongitude) _GPSInfo.longitude = t_lng;
            if (_isAltitude)  _GPSInfo.altitude  = t_alt;
            _GPSInfo.satellites = t_sate;
            _GPSInfo.dop = t_dop;
            _isGGA = false;
            _isLocUpdate = true;
        }
    }
}

void GPS_MTK333X::parser (void) {
    if (_sentence == 0) {
        uint16_t hd = (_nmeaBuff[0] << 8) | _nmeaBuff[1];
        uint16_t ft = (_nmeaBuff[_offset-2] << 8) | _nmeaBuff[_offset-1];
        switch (hd) {
            case 0x504D : {				// PM****
                switch (ft) {
                    case 0x3031 : {		// PMTK001 PMTK_ACK
                        _sentence = _NMEA_PMTK001;
                        return;
                    }
                }
                break;
            }
            case 0x4750 : {				// GP***
                switch (ft) {           // switch (_parity) {
                    case 0x4741 : {     // case 0x7A : { 		// GPGGA
                        _sentence = _NMEA_GGA;
                        _isLatitude = _isLongitude =
                        _isAltitude = _isLocUpdate = false;
                        return;
                    }
                    case 0x4D43 : {     // case 0x67 : {		// GPRMC
                        _isRMC = 0;
                        _sentence = _NMEA_RMC;
                        _isLatitude = _isLongitude =
                        _isTime = _isDate = _isSpeed = _isCourse =
                        _isStatUpdate = false;
                        return;
                    }
                }
                break;
            }
            case 0x474E : {				// GN***
                switch (_parity) {
                    case 0x64 : {		// GNGGA
                        _sentence = _NMEA_GGA;
                        _isLatitude = _isLongitude =
                        _isAltitude = _isLocUpdate = false;
                        return;
                    }
                    case 0x79 : {		// GNRMC
                        _isRMC = 0;
                        _sentence = _NMEA_RMC;
                        _isLatitude = _isLongitude =
                        _isTime = _isDate = _isSpeed = _isCourse =
                        _isStatUpdate = false;
                        return;
                    }
                }
                break;
            }
        }
        _sentence = -1;
        return;
    }

    // 各カラムの解釈
    switch (_sentence) {
        case _NMEA_RMC : {
            switch (_colum) {
                // 時間
                case 1 : {
                    uint32_t temp;
                    if (_offset < 8) {
                        _sentence = -1;
                        _isGGA = false;
                        break;
                    }
                    temp = 0;
                    for (uint8_t i = 0; i < 6; i++) {
                        temp <<= 4;
                        temp += fromHex(_nmeaBuff[i]);
                    }
                    t_time = temp;
                    temp = 0;
                    for (uint8_t i = 8; i < _offset; i++) {
                        temp *= 10;
                        temp += fromHex(_nmeaBuff[i]);
                    }
                    t_millis = temp;
                    _isTime = true;
                    break;
                }
                // RMCステータス
                case 2 : {
                    _isRMC = _nmeaBuff[0] == 'A' ? 2 : 1;
                    break;
                }
                // 緯度
                case 3 : {
                    int32_t temp = fromDegrees(4);
                    t_lat = temp % 1000000;
                    t_lat += ((temp - t_lat) * 3) / 5;
                    break;
                }
                case 4 : {
                    if (_nmeaBuff[0] == 'S') t_lat = -t_lat;
                    _isLatitude = true;
                    break;
                }
                // 経度
                case 5 : {
                    int32_t temp = fromDegrees(4);
                    t_lng = temp % 1000000;
                    t_lng += ((temp - t_lng) * 3) / 5;
                    break;
                }
                case 6 : {
                    if (_nmeaBuff[0] == 'W') t_lng = -t_lng;
                    _isLongitude = true;
                    break;
                }
                // 速度
                case 7 : {
                    t_speed = fromDegrees(2);
                    _isSpeed = true;
                    break;
                }
                // 方位
                case 8 : {
                    t_course = fromDegrees(2);
                    _isCourse = true;
                    break;
                }
                // 日付
                case 9 : {
                    if (_offset == 6) {
                        t_date = 0x20000000L
                            + fromHex(_nmeaBuff[1])
                            + (fromHex(_nmeaBuff[0]) << 4)
                            + ((uint16_t)fromHex(_nmeaBuff[3]) << 8)
                            + ((uint16_t)fromHex(_nmeaBuff[2]) << 12)
                            + ((uint32_t)fromHex(_nmeaBuff[5]) << 16)
                            + ((uint32_t)fromHex(_nmeaBuff[4]) << 20);
                        _isDate = true;
                    }
                    break;
                }
            }
            break;
        }
        case _NMEA_GGA : {
            switch (_colum) {
                case 1 : {
                    if (_offset != 10) {
                        _sentence = -1;
                        _isGGA = false;
                    }
                    break;
                }
                // 緯度
                case 2 : {
                    int32_t temp = fromDegrees(4);
                    t_lat = temp % 1000000;
                    t_lat += ((temp - t_lat) * 3) / 5;
                    break;
                }
                case 3 : {
                    if (_nmeaBuff[0] == 'S') t_lat = -t_lat;
                    _isLatitude = true;
                    break;
                }
                // 経度
                case 4 : {
                    int32_t temp = fromDegrees(4);
                    t_lng = temp % 1000000;
                    t_lng += ((temp - t_lng) * 3) / 5;
                    break;
                }
                case 5 : {
                    if (_nmeaBuff[0] == 'W') t_lng = -t_lng;
                    _isLongitude = true;
                    break;
                }
                // 品質
                case 6 : {
                    if (fromHex(_nmeaBuff[0]) > 0) _isGGA = true;
                    break;
                }
                // 衛星数
                case 7 : {
                    t_sate = fromDegrees(0);
                    break;
                }
                // DOP
                case 8 : {
                    t_dop = fromDegrees(2);
                    break;
                }
                // 標高
                case 9 : {
                    t_alt = fromDegrees(2);
                    _isAltitude = true;
                    break;
                }
            }
            break;
        }
        case _NMEA_PMTK001 : {
            switch (_colum) {
                case 1 : _message = fromDegrees(0); break;
                case 2 : _result  = fromDegrees(0); break;
            }
            break;
        }
    }
}

// from Hex to Decimal digit
uint8_t GPS_MTK333X::fromHex (const uint8_t c) {
    uint8_t d = c | 0x20U;
    if (d & 0x40U) d -= 7U;
    return (d & 0x0FU);
}

// from Degress to Decimal digit
int32_t GPS_MTK333X::fromDegrees (int dec) {
    int32_t degress = 0;
    int16_t point = -1;
    uint8_t sign = false;
    for (uint8_t i = 0; i < _offset; i++) {
        uint8_t c = _nmeaBuff[i];
        if (c == '.') {
            point = 0;
        }
        else if (c == '-') {
            sign = true;
        }
        else if (point < dec) {
            degress = degress * 10 + fromHex(c);
            if (point >= 0) point++;
        }
    }
    if (point < 0) point = 0;
    for (; point < dec; point++) degress *= 10;
    if (sign) degress = -degress;
    return degress;
}

// buildup MTK/UBX command packet
String GPS_MTK333X::createMTKpacket (String command, uint16_t packetType, String dataField) {
    String config = "$P";
    config += command;
    if (packetType < 100) config += '0';
    if (packetType < 10)  config += '0';
    config += packetType;
    if (dataField.length() > 0) config += dataField;
    config += '*';
    config += calcCRCforMTK(config);
    config += "\r\n";
    return (config);
}

// calc CRC
String GPS_MTK333X::calcCRCforMTK (String sentence) {
    uint8_t crc = 0U;
    for (uint8_t x = 1; x < sentence.length() - 1; x++) crc ^= sentence[x];
    String output;
    if (crc < 10U) output += '0';
    output += String(crc, HEX);
    return (output);
}

// bool GPS_MTK333X::resultMTKcommand (bool (*check)(void), uint16_t packetType) {
//     _message = -1;
//     uint32_t ms = millis();
//     while ((millis() - ms) < 1000U) {
//         if (check() && _message == packetType) return (_result == 3U);
//     }
//     return false;
// }

// end of code
