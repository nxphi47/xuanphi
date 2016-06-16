import socket

port = 12345
ip = socket.gethostname()

if __name__ == '__main__':
	# make socket and connect to local machine
	s = socket.socket()
	try:
		s.connect((ip, port))
	except:
		print "Failed to connect %s" %(ip)
		exit(1)

	# loop
	print "Connection completed"
	while True:
		exp = raw_input("Enter expression: ")
		if str(exp).lower() == "close":
			s.close()
			break
		else:
			s.send(str(exp))
			result = s.recv(1024)
			if result:
				print "Result: ", result
			else:
				print "Disconnected !"
				break

	print "program finish"
