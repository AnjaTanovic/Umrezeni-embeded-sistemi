from socket import *
import time
import mysql.connector
import json

mydb = mysql.connector.connect(
  host = "localhost",
  user = "prezime.exx",
  password = "...",
  database = "db_prezime_exx"
)

port = 50061
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', port))
print ('UDP server (port ' + str(port) + ')\n')

msgCnt = 1
while True:
    try:
        messageIn, clientAddress = serverSocket.recvfrom(4096)
        
        ts = time.localtime()
        print('\033[0;34;40mUDP server (' + str(port) + ') Poruka#', str(msgCnt))
        print(time.strftime('%Y-%m-%d %H:%M:%S\033[0;37;40m', ts))
        
        data = json.loads(messageIn.decode("utf-8"))
        print ("   Temperatura:     ", data["temp"]["value"], "\u00b0C")
        print ("   Pritisak:        ", data["pres"]["value"], "mBar")
        print ("   Vlaznost vazduha:", data["hum"]["value"], "%")
        print ("   Osvetljenost:    ", data["lum"]["value"], "lux")
        
        mycursor = mydb.cursor()
        mycursor.execute("INSERT INTO merenja (tip_senzora, vrednost) VALUES ('temperatura'," + str(data["temp"]["value"]) + ")")
        mycursor.execute("INSERT INTO merenja (tip_senzora, vrednost) VALUES ('pritisak'," + str(data["pres"]["value"]) + ")")
        mycursor.execute("INSERT INTO merenja (tip_senzora, vrednost) VALUES ('vlaznost'," + str(data["hum"]["value"]) + ")")
        mycursor.execute("INSERT INTO merenja (tip_senzora, vrednost) VALUES ('osvetljenost'," + str(data["lum"]["value"]) + ")")
        mydb.commit()
        
        serverSocket.sendto(bytearray("OK!", "utf-8"), clientAddress)
        
        msgCnt += 1
    except:
        print('Greska!!!')
