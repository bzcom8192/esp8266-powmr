#ifndef __VERSION_H
#define __VERSION_H

#if defined(ESP8266)
const String esp            = "ESP8266";
#endif

const String model          = "esp8266-powmr";
const String version        = "0.0.2";
const String built          = String(__DATE__) + " " + String(__TIME__);

#endif // __VERSION_H