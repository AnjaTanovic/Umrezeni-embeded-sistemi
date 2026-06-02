from socket import *
import time

port = 50123
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', port))
print('Echo server is ready to receive (port ' + str(port) + ')\n')

msgCnt = 1
while True:
	try:
		messageIn, clientAddress = serverSocket.recvfrom(4096)
		ts=time.localtime()
		print('\033[0;34;40mEcho server (' + str(port) + ') Msg#', str(msgCnt))
		print(time.strftime('%Y-%m-%d %H:%M:%S\033[0;37;40m',ts))
		print(' Rx: ',messageIn)
		print(' Tx: ',messageIn.upper())
		serverSocket.sendto(messageIn.upper(), clientAddress)
		
		msgCnt += 1
	except:
		print('ERROR in Echo UDP')