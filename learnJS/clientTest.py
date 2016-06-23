import socket
import traceback
import sys
import time
import select
import BaseHTTPServer
import CGIHTTPServer
import thread

port = 5559
addressSocketServer = ("", port)

httpPort = 5558
addresHTTPServer = ("", port)


############ practice making server

def makeSocketServer():
	# create the socket
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# make the socket reuseable after turn off without normal timeout
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	# bind the socket with designated address
	s.bind(addressSocketServer)
	s.listen(5)

	return s


def makeHTTPServer():
	_server = BaseHTTPServer.HTTPServer
	_handler = CGIHTTPServer.CGIHTTPRequestHandler
	_handler.cgi_directories("/cgi")
	httpd = _server(addresHTTPServer, _handler)
	httpd.serve_forever()


if __name__ == '__main__':
	sockServer = makeSocketServer()
	try:
		thread.start_new_thread(makeHTTPServer, ())
		inputs = [sockServer]
		outputs = []
		while sockServer in inputs:
			readable, writable, exceptional = select.select(inputs, outputs, inputs)

			for sock in readable:
				if sock is sockServer:
					client, address = sock.accept()
					client.setblocking(0)
					# add the client to inputs for select
					inputs.append(client)
				else:
					data = sock.recv(4096)
					if data:
						# data is avalable
						# do somthing on the data

						# add the sock to outputs for ready to send
						if data not in outputs:
							outputs.append(data)
					else:
						# disconnection
						sock.close()
						inputs.remove(sock)
						if sock in outputs:
							outputs.remove(sock)

			# handle writable
			for sock in writable:
				# process data for writable
				data = "hello"
				sock.send()
				# if nothing to send, remove the socket from output
		sockServer.close()

	except KeyboardInterrupt:
		print "Keybouard interrupt, shutdown"
	except:
		traceback.print_exc(file=sys.stdout)
	finally:
		sockServer.close()