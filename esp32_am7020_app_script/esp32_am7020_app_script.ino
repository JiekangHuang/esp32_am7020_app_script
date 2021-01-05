#include "config.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

#include <TinyGsmClient.h>
#include <SSLClient.h>
#include <HTTPSRedirect.h>

#include "AllTrustAnchors.h"

SoftwareSerial mhz19bSerial, zh03bSerial;

// mhz219b uart read command
const uint8_t mhz19b_read_concentration[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
// zh03b uart set qa mode command
const uint8_t zh03b_set_qa_mode[] = {0xff, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46};
// zh03b uart read command
const uint8_t zh03b_read_data[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger, AM7020_RESET);
#else
// 建立 AM7020 modem（設定 Serial 及 EN Pin）
TinyGsm modem(SerialAT, AM7020_RESET);
#endif

// 建立 AM7020 TCP Client
TinyGsmClient tcpClient(modem);
// 建立 SSL Client，下層接 TCP Client
SSLClient     sslClient(tcpClient, TAs, (size_t)TAs_NUM, A0);
// 建立 HTTP Client，下層接 SSL Client
HTTPSRedirect httpClient(sslClient);

int  readMHZ19BCO2(void);
int  readZH03BPM25(void);
void setZH03BQAMode(void);

void nbConnect(void);

void setup()
{
    SerialMon.begin(MONITOR_BAUDRATE);
    SerialAT.begin(AM7020_BAUDRATE);

    // AM7020 NBIOT 連線基地台
    nbConnect();
    // 建立  mhz19b Software Serial(rx_pin: 18, tx_pin: 19)
    mhz19bSerial.begin(BAUDRATE_9600, SWSERIAL_8N1, MHZ19B_RX, MHZ19B_TX, false, MHZ19B_BUFF_SIZE, 11);
    // 建立 zh03b Software Serial(rx_pin: 5, tx_pin: 23)
    zh03bSerial.begin(BAUDRATE_9600, SWSERIAL_8N1, ZH03B_RX, ZH03B_TX, false, ZH03B_BUFF_SIZE, 11);
    setZH03BQAMode();
}

void loop()
{
    static unsigned long timer = 0;
    int                  co2, pm25;
    char                 buff[300];
    int                  state_code;
    String               body;

    // 讀取 co2 & pm2.5
    co2  = readMHZ19BCO2();
    pm25 = readZH03BPM25();


    if (millis() >= timer) {
        timer = millis() + UPLOAD_INTERVAL;
        // 檢查 NBIOT 連線狀態
        if (!modem.isNetworkConnected()) {
            nbConnect();
        }
        SerialMon.print(F("co2 : "));
        SerialMon.println(co2);
        SerialMon.print(F("pm25: "));
        SerialMon.println(pm25);

        if (co2 > 0 && pm25 > 0) {
            SerialMon.println(F("HTTP Get..."));
            sprintf(buff, HTTP_API, String(pm25).c_str(), String(co2).c_str());
            httpClient.connect(HTTP_SERVER, HTTP_PORT);
            httpClient.GET(buff);
            state_code = httpClient.getStatusCode();
            body       = httpClient.getResponseBody();

            SerialMon.print(F("GET state code = "));
            SerialMon.println(state_code);
            SerialMon.print(F("body = "));
            SerialMon.println(body);

            httpClient.stop();
        }
    }
}

/**
 * AM7020 NBIOT 連線基地台
 */
void nbConnect(void)
{
    SerialMon.println(F("Initializing modem..."));
    // 初始化 & 連線基地台
    while (!modem.init() || !modem.nbiotConnect(APN, BAND)) {
        SerialMon.print(F("."));
    };

    SerialMon.print(F("Waiting for network..."));
    // 等待網路連線
    while (!modem.waitForNetwork()) {
        SerialMon.print(F("."));
    }
    SerialMon.println(F(" success"));
}

/**
 * 讀取 MHZ219B 二氧化碳濃度
 * @return 二氧化碳濃度
 */
int readMHZ19BCO2(void)
{
    uint8_t       data[9], check_sum = 0x00;
    unsigned long timer;
    // write command to mhz19b
    mhz19bSerial.write(mhz19b_read_concentration, sizeof(mhz19b_read_concentration));
    mhz19bSerial.flush();

    timer = millis();
    while (mhz19bSerial.available() < 9 && millis() - timer < 1000)
        ;

    // read response
    if (mhz19bSerial.available() >= 9) {
        // read all data
        for (int ii = 0; ii < 9; ii++) {
            data[ii] = mhz19bSerial.read();
        }

        // calc check sum
        for (int ii = 1; ii < 8; ii++) {
            check_sum += data[ii];
        }
        // check sum
        if ((((check_sum & 0xFF) ^ 0xFF) + 1) != data[8]) {
            return -1;
        }
        return ((data[2] << 8) | data[3]);
    }
    return -2;
}

/**
 * 讀取 ZH03B PM2.5
 * @return PM2.5
 */
int readZH03BPM25(void)
{
    uint8_t       data[9], check_sum = 0x00;
    unsigned long timer;
    // write command to ZH03B
    zh03bSerial.write(zh03b_read_data, sizeof(zh03b_read_data));
    zh03bSerial.flush();

    timer = millis();
    while (zh03bSerial.available() < 9 && millis() - timer < 1000)
        ;

    // read response
    if (zh03bSerial.available() >= 9) {
        // read all data
        for (int ii = 0; ii < 9; ii++) {
            data[ii] = zh03bSerial.read();
        }

        // calc check sum
        for (int ii = 1; ii < 8; ii++) {
            check_sum += data[ii];
        }
        // check sum
        if ((((check_sum & 0xFF) ^ 0xFF) + 1) != data[8]) {
            return -1;
        }
        return ((data[2] << 8) | data[3]);
    }
    return -2;
}

/**
 * 設定 ZH03B 資料接收為被動模式
 */
void setZH03BQAMode(void)
{
    // write command to ZH03B
    zh03bSerial.write(zh03b_set_qa_mode, sizeof(zh03b_set_qa_mode));
    zh03bSerial.flush();
}
