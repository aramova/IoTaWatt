#pragma once
#include <Arduino.h>

class xbuf;

void base64encode(xbuf* buf);
String base64encode(const uint8_t* in, size_t len);
