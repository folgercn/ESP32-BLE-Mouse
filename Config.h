#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Set to 1 to enable Serial debug output; set to 0 to strip logs at compile time.
#ifndef DEBUG_LOG_ENABLED
#define DEBUG_LOG_ENABLED 1
#endif

#if DEBUG_LOG_ENABLED
#define DEBUG_SERIAL_BEGIN(...) Serial.begin(__VA_ARGS__)
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_SERIAL_BEGIN(...) ((void)0)
#define DEBUG_PRINT(...) ((void)0)
#define DEBUG_PRINTLN(...) ((void)0)
#define DEBUG_PRINTF(...) ((void)0)
#endif

#endif
