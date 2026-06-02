import socket 
HOST = ''
PORT = 50123
NUM_OF_CLIENTS = 1
tcpSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcpSocket.bind((HOST, PORT))
tcpSocket.listen(NUM_OF_CLIENTS)
print('Echo server is ready to receive (port ' + str(PORT) + ')\n')

msgCnt = 1
while True:
	try:
		conn, addr = tcpSocket.accept()
		print('Connected by', addr)
		while True:
			data_in = conn.recv(1024)
			if not data_in:
				break
			data_out = data_in.upper()
			print(' Rx: ',data_in)
			print(' Tx: ',data_out)
			conn.sendall(data_out)
	except:
		conn.close()
		print('ERROR in Echo TCP')