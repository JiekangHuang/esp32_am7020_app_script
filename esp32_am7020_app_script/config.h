#ifndef _CONFIG_H
#define _CONFIG_H
#include <Arduino.h>

#define TINY_GSM_MODEM_SIM7020
/* Define the serial console for debug prints, if needed */
// #define TINY_GSM_DEBUG SerialMon
/* uncomment to dump all AT commands */
// #define DEBUG_DUMP_AT_COMMAND

/* baudrate */
#define BAUDRATE_9600 9600
#define BAUDRATE_19200 19200
#define BAUDRATE_38400 38400
#define BAUDRATE_115200 115200

/* software Serial config */
#define MHZ19B_RX 18
#define MHZ19B_TX 19
#define MHZ19B_BUFF_SIZE 32

#define ZH03B_RX 5
#define ZH03B_TX 23
#define ZH03B_BUFF_SIZE 32


#define UPLOAD_INTERVAL (15 * 60000)

#define SerialMon Serial
#define MONITOR_BAUDRATE BAUDRATE_115200

/* ESP32 Boards */
#define SerialAT Serial2
#define AM7020_BAUDRATE BAUDRATE_115200
#define AM7020_RESET 4

/* for taiwan mobile */
#define APN "twm.nbiot"
#define BAND 28

/* apps script api config */
#define HTTP_PORT 443
#define HTTP_SERVER "script.google.com"
#define APPS_SCRIPT_ID "<YOUR APPS SCRIPT ID>"
#define GOOGLE_SHEET_ID "<YOUR GOOGLE SHEET ID>"
#define HTTP_API "/macros/s/" APPS_SCRIPT_ID "/exec?id=" GOOGLE_SHEET_ID "&pm25=%s&co2=%s"

#endif /* _CONFIG_H */
