#include "base64.h"
#include "trace.h"
#include "xbuf.h"

const char base64codes_P[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/**************************************************************************************************
 * Convert the contents of an xbuf to base64
 * ************************************************************************************************/
void base64encode(xbuf* buf){
  trace(T_base64,10);
  char* base64codes = new char[72];
  if( ! base64codes){
      trace(T_base64,11);
  }
  strcpy_P(base64codes, base64codes_P);
  size_t supply = buf->available();
  uint8_t in[3];
  uint8_t out[4];
  trace(T_base64,14,supply);
  while(supply >= 3){
    buf->read(in,3);
    out[0] = (uint8_t) base64codes[in[0]>>2];
    out[1] = (uint8_t) base64codes[(in[0]<<4 | in[1]>>4) & 0x3f];
    out[2] = (uint8_t) base64codes[(in[1]<<2 | in[2]>>6) & 0x3f];
    out[3] = (uint8_t) base64codes[in[2] & 0x3f];
    buf->write(out, 4);
    supply -= 3;
  }
  trace(T_base64,15,supply);
  if(supply > 0){
    in[0] = in[1] = in[2] = 0;
    buf->read(in,supply);
    out[0] = (uint8_t) base64codes[in[0]>>2];
    out[1] = (uint8_t) base64codes[(in[0]<<4 | in[1]>>4) & 0x3f];
    out[2] = (uint8_t) base64codes[(in[1]<<2 | in[2]>>6) & 0x3f];
    out[3] = (uint8_t) base64codes[in[2] & 0x3f];
    if(supply == 1) {
      out[2] = out[3] = (uint8_t) '=';
    }
    else if(supply == 2){
      out[3] = (uint8_t) '=';
    }
    buf->write(out, 4);
  }
  trace(T_base64,16);
  delete[] base64codes;
}

String base64encode(const uint8_t* in, size_t len){
  trace(T_base64,0,len);
  if(len <= 0){
     trace(T_base64,1);
     return String("");
  }
  xbuf work(128);
  work.write(in, len);
  trace(T_base64,3,len);
  base64encode(&work);
  trace(T_base64,4);
  //Serial.printf("base64 %s %d\n",work.peekString().c_str(), ESP.getFreeHeap());
  String result = work.readString(work.available());
  return result;
}
