/*
 * Quectel_BG600L.c
 *
 * Created: 16-Oct-20 10:49:45 AM
 *  Author: Milan Lukic
 */

#include "Quectel_BG600L.h"

bool DEBUG_ENABLED = true;
char IMSI[20] = "";
char response[1400];

void BG600L_debugEnable(bool debugEnabled)
{
	DEBUG_ENABLED = debugEnabled;
}

void BG600L_activatePins(void)
{
  pinMode(BG95_PWR, OUTPUT);
}

void BG600L_deactivatePins(void)
{
  pinMode(BG95_PWR, INPUT);
}

bool BG600L_turnON(void)
{
  NBIOT_STREAM.begin(115200, SERIAL_8N1, BG95_RXD, BG95_TXD);
  delay(1000);

  BG600L_activatePins();

	char response [256];
	if (getBG600Lresponse("AT\r\n", "OK", response, 1000, false))
	{
		DEBUG_STREAM.print("BG600L ON!\r\n");
		return true;
	}

	bool turn_on = false;
	do
	{
		DEBUG_STREAM.print("BG600L reset... ");
	
		//BG600L reset
    digitalWrite(BG95_PWR, HIGH);
    delay(1000);
    digitalWrite(BG95_PWR, LOW);

		DEBUG_STREAM.print("Done!\r\n");
		turn_on = getBG600Lresponse("", "RDY", response, 8000, false);
	} 
	while (!turn_on);
	
	if (!getBG600Lresponse("ATI\r\n", "APP RDY", response, 8000, false))
		return false;
	
	return true;
}

BG600L_CONNECTION_STATUS BG600L_pollConnectionStatus(uint8_t attempts, uint16_t retry_interval)
{
	char response [256];
	bool connect_flag = false;
	
	for (uint8_t i = 0; i < attempts; i++)
	{
		getBG600Lresponse("AT+QCSQ\r\n", "OK", response, 1000, false);
		connect_flag = getBG600Lresponse("AT+CGATT?\r\n", "+CGATT: 1", response, retry_interval, false);
		if (connect_flag)
			break;
	}

	if (!connect_flag)
		return NOT_CONNECTED;

	if (!getBG600Lresponse("AT+QCSQ\r\n", "OK", response, 1000, false))
		return NOT_CONNECTED;

	if (strstr(response, "GSM"))
		return GSM_CONNECTED;
	if (strstr(response, "NBIoT"))
		return NB_IOT_CONNECTED;
	if (strstr(response, "eMTC"))
		return EMTC_CONNECTED;
	
	return NOT_CONNECTED;
}

char * BG600L_getIMSI()
{
	char response[256];

	//get IMSI
	if (!getBG600Lresponse("AT+CIMI\r\n", "OK", response, 2000, false))
		return NULL;
	memcpy(IMSI, strstr(response, "\r\n") + 2, 15);
	IMSI[16] = '\0';
	sprintf(response, "IMSI: %s\r\n", IMSI);
	DEBUG_STREAM.print(response);

	return IMSI;
}

bool BG600L_nwkSetup()
{
	char cmd[64], response[256];
  
  if (!getBG600Lresponse("AT\r\n", "OK", response, 3000, false))
    return false;
  //Configure the module to use NB-IoT (if not already set):
//  if (!getBG600Lresponse("AT+QCFG=\"nwscanmode\",3,1\r\n", "OK", response, 3000, false))
//    return false;
  //Set the preferred mode to CAT-NB (NB-IoT):
  if (!getBG600Lresponse("AT+QCFG=\"iotopmode\",1\r\n", "OK", response, 3000, false))
    return false;
  //Select the appropriate frequency bands (B20):
  if (!getBG600Lresponse("AT+QCFG=\"band\",0,0x80000,0\r\n", "OK", response, 3000, false))
    return false;
  //Configure APN (Access Point Name) for the network:
  if (!getBG600Lresponse("AT+CGDCONT=1,\"IP\",\"iot\"\r\n", "OK", response, 3000, false))
    return false;
  //Register to the network:
  if (!getBG600Lresponse("AT+COPS=0\r\n", "OK", response, 3000, false))
    return false;

  return BG600L_pollConnectionStatus(30, 3000);
}

bool BG600L_TxUDPbin(char server_IP[], uint16_t port, uint8_t payload[], uint16_t size, bool *tx_response)
{   
	char cmd[128];
	*tx_response = false;
	
	BG600L_CONNECTION_STATUS connection_status = BG600L_pollConnectionStatus(1, 3000);
	
	if (connection_status == NOT_CONNECTED)
	{
		DEBUG_STREAM.printf("enter wake sequence\n");
		BG600L_turnON();
	}

	if (!getBG600Lresponse("AT+QIOPEN=1,2,\"UDP SERVICE\",\"127.0.0.1\",0,3030,0\r\n", "+QIOPEN: 2,0", response, 8000, false))
	{
		getBG600Lresponse("AT+QICLOSE=2\r\n", "OK", response, 8000, false);
		return false;
	}

	if (!getBG600Lresponse("AT+QISTATE=0,1\r\n", "OK", response, 8000, false))
		return false;
	
	getBG600Lresponse("AT+QCSQ\r\n", "OK", response, 1000, false);
	sprintf(cmd, "AT+QISEND=2,%d,\"%s\",%d\r\n", size, server_IP, port);
	if (!getBG600Lresponse(cmd, ">", response, 8000, false))
	{
		getBG600Lresponse("AT+QICLOSE=2\r\n", "OK", response, 8000, false);
		return false;
	}
	for (uint16_t i = 0; i < size; i++)
	{
		if (i == size - 1)
			sprintf(cmd, "%02X\r\n", payload[i]);
		else
			sprintf(cmd, "%02X", payload[i]);
		DEBUG_STREAM.print(cmd);
		NBIOT_STREAM.print((char)payload[i]);
	}

	DEBUG_ENABLED = true;
	uint8_t resp = getBG600Lresponse("\r\n", "SEND OK", response, 120000, true);

	if (resp == RESP_ERROR)
	{
		DEBUG_ENABLED = true;
		getBG600Lresponse("AT+QICLOSE=2\r\n", "OK", response, 8000, false);
		return false;
	}
	else if (resp == RESP_OK_AND_QUIRC)
	{
		*tx_response = true;
	}
	else
	{
		DEBUG_ENABLED = true;
		DEBUG_STREAM.print("SEND OK!\r\n");
	}
	return true;
}

void BG600L_wakeup()
{
	bool turn_on = false;
	do
	{
		DEBUG_STREAM.print("BG600L wakeup... ");
	
		//BG600L reset
    digitalWrite(BG95_PWR, HIGH);
    delay(1000);
    digitalWrite(BG95_PWR, LOW);
		DEBUG_STREAM.print("Done!\r\n");
		//getBG600Lresponse("AT+CPSMS=0\r\n", "OK", response, 8000, false);
		turn_on = getBG600Lresponse("", "RDY", response, 8000, false);
	} 
	while (!turn_on);
	
}

int BG600L_RxUDPbin(uint8_t payload[], bool tx_response)
{
	char cmd[128];
	
	if (!tx_response)
	{
		if (!getBG600Lresponse("", "+QIURC: \"recv\",", response, 5000, false))
		{
			DEBUG_ENABLED = true;
			getBG600Lresponse("AT+QICLOSE=2\r\n", "OK", response, 8000, false);
			return 0;
		}
	}

	//int connectID = atoi(strstr(response, "+QIURC: \"recv\",") + sizeof("+QIURC: \"recv\",") - 1);
	int connectID = 2;
	sprintf (cmd, "AT+QIRD=%d\r\n", connectID);
	DEBUG_STREAM.printf("NB-IoT CMD -> %s", cmd);

	DEBUG_ENABLED = false;
	if (!getBG600Lresponse(cmd, "OK", response, 8000, false))
	{
		DEBUG_ENABLED = true;
		sprintf(cmd, "AT+QICLOSE=%d\r\n", connectID);
		getBG600Lresponse(cmd, "OK", response, 2000, false);
		return 0;
	}
	DEBUG_ENABLED = true;

	char *tmp = strstr(response, "+QIRD:");
	uint16_t dlSize = atoi(tmp + sizeof("+QIRD:"));
	DEBUG_STREAM.printf("DL size = %d\r\n", dlSize);

	do
	{
		DEBUG_STREAM.printf("%c", *tmp);
	}
	while (*tmp++ != '\n');
	for (int i = 0; i < dlSize; i++)
		//DEBUG_STREAM.printf("%02X", tmp[i]);
		DEBUG_STREAM.print(tmp[i]);
	DEBUG_STREAM.printf("\r\n");

	if (dlSize > 0)
		memcpy(payload, tmp, dlSize);
	else
		payload = NULL;

	getBG600Lresponse("AT+QICLOSE=2\r\n", "OK", response, 2000, false);
	
	return dlSize;
}

void BG600L_enterPSM()
{
	char response[256];
	getBG600Lresponse("AT+QPSMS=1,,,\"01000001\",\"00001111\"\r\n", "OK", response, 5000, false);
	//getBG600Lresponse("AT+CFUN=0\r\n", "OK", response, 5000, false);

	if (!getBG600Lresponse("AT+QCFG=\"psm/enter\",1\r\n", "PSM POWER DOWN", response, 30000, false))
		getBG600Lresponse("AT+QPOWD=1\r\n", "OK", response, 3000, false);
}

void BG600L_powerDown()
{
  char response[256];
	getBG600Lresponse("AT+QPOWD=1\r\n", "POWERED DOWN", response, 8000, false);
}

bool BG600L_timeSync()
{
	char response [256];
	char *tmp;
	struct tm tm;
	int t_zone = 0;
	time_t epoch;

	if (!getBG600Lresponse("AT+QNTP=1,\"pool.ntp.org\",123\r\n", "OK", response, 125000, false))
	{
		return false;
	}
		

	if (!getBG600Lresponse("AT+CCLK?\r\n", "OK", response, 1000, false))
		return false;
	
	tmp = strstr(response, "+CCLK:");
	if (!tmp)
		return false;
	
	tmp += 8;

	tm.tm_year = atoi(tmp) + 100; // since 1900
	tmp += 3;
	tm.tm_mon = atoi(tmp) - 1;
	tmp += 3;
	tm.tm_mday = atoi(tmp);
	tmp += 3;
	tm.tm_hour = atoi(tmp) - 1;
	tmp += 3;
	tm.tm_min = atoi(tmp);
	tmp += 3;
	tm.tm_sec = atoi(tmp);
	tmp += 3;

	if (*(tmp - 1) == '+')
		t_zone = atoi(tmp);
	else
		t_zone = -1 * (atoi(tmp));

	// get UNIX timestamp
	epoch = mktime(&tm);

	uint64_t time_ms = ((uint64_t)epoch)*1000;
	
	struct timeval tv;
    tv.tv_sec = time_ms / 1000;
    tv.tv_usec = (time_ms % 1000) * 1000;

	//DEBUG_STREAM.printf("sec : %ld\n", tv.tv_sec);
    settimeofday(&tv, NULL); // upis u rtc
	
	// set time zone
	setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1); // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  	tzset();

    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo); // ispis
	DEBUG_STREAM.printf("UNIX TS: %ld\n", (long int)tv.tv_sec);
    DEBUG_STREAM.printf("Current date/time is: %04d-%02d-%02d %02d:%02d:%02d \n",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	return true;
}

time_t BG600L_getUnixTS()
{
	time_t now;
	time(&now);

	DEBUG_STREAM.printf("Time: %ld\n", (long int)now);

	return now;
}


uint8_t getBG600Lresponse(char command[], char exp_response[], char response[], uint32_t timeout, bool catch_downlink)
{
	uint16_t count = 0;
	char quirc[] = "+QIURC: \"recv\",";
	bool resp_OK = false;
	bool catch_ok = false;

	response[0] = '\0';
	
	if (DEBUG_ENABLED)
	{
		DEBUG_STREAM.print("NB-IoT CMD -> ");
		DEBUG_STREAM.print(command);
	}

	NBIOT_STREAM.print(command);
	
	uint64_t t0 = millis();
	while ((millis() - t0) < timeout)
	{
		if (NBIOT_STREAM.available())
		{
			response[count] = NBIOT_STREAM.read();
			if (DEBUG_ENABLED)
				DEBUG_STREAM.printf("%c", response[count]);
			else
				delay(2);
			response[++count] = '\0';
		}
		if (catch_downlink)
		{
			if (strstr(response, quirc))
			{
				catch_ok = true;
				//break;
			}
		}
		
		if (strstr(response, exp_response))
		{
			resp_OK = true;
			break;
		}
	}
	
	delay(20);
	while (NBIOT_STREAM.available())
	{
		//delay(1);
		response[count] = NBIOT_STREAM.read();
		if (DEBUG_ENABLED)
			DEBUG_STREAM.printf("%c", response[count]);
		response[++count] = '\0';
		if (strstr(response, quirc))
		{
			catch_ok = true;
			//break;
		}
	}
	
	if (DEBUG_ENABLED)
	{
		//Crypto_debugPrint((int8_t *)"response: ", (uint8_t *)response, count);
	}
	
	if (catch_downlink)
	{
		if (resp_OK && !catch_ok)
			return RESP_OK_BUT_NO_QUIRC;
		else if (resp_OK && catch_ok)
			return RESP_OK_AND_QUIRC;
	}
	else
	{
		if (resp_OK)
			return RESP_OK;
	}

	return RESP_ERROR;
}


void BG600L_serialBridge(void)
{
	char str[256];
	DEBUG_STREAM.printf("Serial bridge...\r\n");
	
	while (1)
	{
		if(DEBUG_STREAM.available())
		{
			delay(100);
			int len = DEBUG_STREAM.available();
		  DEBUG_STREAM.readBytes(str, len);
			
			if (strstr(str, "EXIT"))
			{
				DEBUG_STREAM.printf("Exit bridge mode...\r\n");
				break;
			}
			
			DEBUG_STREAM.printf("CMD -> ");
			DEBUG_STREAM.printf(str);
			
			NBIOT_STREAM.print(str);
		}
		
		if(NBIOT_STREAM.available())
			DEBUG_STREAM.write(NBIOT_STREAM.read());
	}
}
