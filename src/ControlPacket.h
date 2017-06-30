#ifndef ControlPackage_h
#define ControlPackage_h
#include <Arduino.h>

struct ControlPacket {
 byte Id;
 byte Command;
 byte Value1 ;
 byte Value2 ;
 byte Value3 ;
 byte Value4 ;
 byte Value5 ;
};

#endif
