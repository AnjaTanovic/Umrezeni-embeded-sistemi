/*
 * Quectel_BG600L.h
 *
 * Created: 16-Oct-20 10:50:16 AM
 *  Author: Milan Lukic
 */ 

#include <Arduino.h>

#ifndef QUECTEL_BG600L_H_
#define QUECTEL_BG600L_H_

#define DEBUG_STREAM  Serial
#define NBIOT_STREAM  Serial2

#define BG95_PWR 4
#define BG95_RXD 33
#define BG95_TXD 32

typedef enum {NOT_CONNECTED, GSM_CONNECTED, NB_IOT_CONNECTED, EMTC_CONNECTED} BG600L_CONNECTION_STATUS;

// error codes
/***********************************/
#define RESP_ERROR 0x00
#define RESP_OK 0x01
#define RESP_OK_BUT_NO_QUIRC 0x02
#define RESP_OK_AND_QUIRC 0x03
/***********************************/

/**
* Function used to enable printing of debug messages.
* @param debugEnabled - boolean value, where true enables debug messages
* @return - no return value
*/
void BG600L_debugEnable(bool debugEnabled);
/**
* Function used for sending AT commands to BG600L module and recieving response
* @param command - pointer to buffer where AT command is stored
* @param exp_response - pointer to buffer where expected response from BG600L is stored
* @param response - pointer to buffer where response from BG600L will be saved
* @param timeout - integer value that represents time during response is expected to appear
* @return -  true if operation is successful, meaning that expected response has been recieved, otherwise false
*/
uint8_t getBG600Lresponse(char command[], char exp_response[], char response[], uint32_t timeout, bool catch_downlink);
/**
* Function used to activate pins necessary for successful function of BG600L module
* @return - no return value
*/
void BG600L_activatePins(void);
/**
* Function used to deactivate pins necessary for function of BG600L module
* @return - no return value
*/
void BG600L_deactivatePins(void);
/**
* Function used to reset BG600L module
* @return - true if operation is successful, otherwise false
*/
bool BG600L_turnON(void);
/**
* Function used to syncronise RTC timer to network time
* @return - true if operation is successful, otherwise false
*/
bool BG600L_timeSync(void);

/**
 * @brief Function polls the BG600L connection status
 * 
 * @param attempts 
 * @param retry_interval 
 * @return BG600L_CONNECTION_STATUS 
 */
BG600L_CONNECTION_STATUS BG600L_pollConnectionStatus(uint8_t attempts, uint16_t retry_interval);

/**
 * @brief Read the IMSI from the SIM card
 */
char * BG600L_getIMSI();

/**
* Function used to attach BG600L module to NB-IoT network
* @return - true if operation is successful, otherwise false
*/
bool BG600L_nwkSetup();
/**
* Function used to transmit packet to the server via NB-IoT network using UDP protocol
* @param server_IP - pointer to buffer where server IP address is stored
* @param port - integer value of server port
* @param payload - pointer to buffer where packet is stored
* @param size - integer value that represents number of bytes to be transmitted
* @return - true if operation is successful, otherwise false
*/
bool BG600L_TxUDPbin(char server_IP[], uint16_t port, uint8_t payload[], uint16_t size, bool *tx_response);

/**
 * Function used to read the incoming UDP downlink packet 
 * 
 * @param payload - pointer to buffer where packet is stored
 * @return number of bytes in received packet
 */
int BG600L_RxUDPbin(uint8_t payload[], bool tx_response);

/**
* Function used to get RTC timer value
* @return - RTC timer value
*/
time_t BG600L_getUnixTS();

void BG600L_enterPSM();
void BG600L_powerDown();
void BG600L_wakeup();

void BG600L_serialBridge(void);

#endif /* QUECTEL_BG600L_H_ */