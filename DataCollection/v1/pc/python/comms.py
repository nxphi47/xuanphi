#!/bin/env python2

import socket
import select
import time

MSG_TYPES = {
	'INIT': 0x02, 'INIT-ACK': 0x03,
	'FINI': 0x00, 'FINI-ACK': 0x01,
	'GET-STATE': 0x04, 'SET-STATE': 0x06, 'STATE-ACK': 0x05,
	'READ': 0x08, 'DATA': 0x09,
	'KEEPALIVE': 0x0a,
	'GET-TIMING': 0x0c, 'SET-TIMING': 0x0e, 'TIMING-ACK': 0x0d,
}

__START = time.time()

# obtain current timestamp
def get_curr_timestamp(st=None):
	if st != None:
		return int((st - __START) * 1000) & 0xffff
	else:
		return int((time.time() - __START) * 1000) & 0xffff

# Socket Buffer
#	Use to act as a buffer to hold partially received /sent messages
class SockBuf(object):
	def __init__(self):
		self.reset()
		
	# reset the buffer
	def reset(self, elen=0, buf=''):
		self.elen = elen		# expected received length
		self.clen = 0			# current sent length
		self.buf = buf
		
	# receive data to the buffer
	# the elen field should have been set prior to calling recv()
	def recv(self, sock):
		while (self.elen > len(self.buf)):
			m = sock.recv(self.elen - len(self.buf))
			self.buf += m
			if (m == ''):
				return (-1)
		return (0)
	
	# send remaining data in the buffer
	# the buf should have been stored prior to calling send()
	# Returns -1 on error, return remaining length to be sent otherwise
	def send(self, sock):
		if (self.clen < len(self.buf)):
			r = sock.send(self.buf[self.clen:])
			if (r==0):
				return -1
			self.clen += r
		return (len(self.buf) - self.clen)
		
# Communications Message Class
#	This class hides the complexity of cunderlying bytes communications from the user
#	To prepare a message to be sent, simplay call:
#		msg = CommsMsg(commsock, ('STATE-ACK', [sen[0].state, sen[1].state, sen[2].state, sen[3].state]))
#	for example to prepapre a STATE-ACK message with the array of sensors states
#	Those messages that require current timestamp will be automatically inserted, 
#	hence caller need not specify those.
#	For KEEP-ALIVE, you need to specify if you want to send timestamp (0=no, 1=yes)
#	commsock is a CommsSock() instance
class CommsMsg(object):
	def __init__(self, msg=None):
		self._buf = ''
		if (msg != None):
			self.set_msg(msg)

	# Prepare this message for sending
	# msg will be a tuple containing the message type and paramaters if any.
	#	Examples:
	#		msg.set_msg(('INIT',))
	#		msg.set_msg(('INIT-ACK',))	# Timestamp will automatically be inserted, no need to specify
	#		msg.set_msg(('DATA', sen[0].id, sen[0].sample_ts, sen[0].samples))
	#		msg.set_msg(('KEEPALIVE', 0))	# 0 inidcate not to insert timestamp
	def set_msg(self, msg):
		if type(msg) == type('INIT'):
			self.mtype = MSG_TYPES[msg]
			self.mlen = 0
			self._buf = self._pack_header(self.mtype, self.mlen)
			return (self._buf)			
		if (msg[0] in ['INIT', 'FINI', 'FINI-ACK', 'GET-STATE', 'READ', 'GET-TIMING']):
			self.mtype = MSG_TYPES[msg[0]]
			self.mlen = 0
			self._buf = self.__pack_header(self.mtype, self.mlen)
			return (self._buf)
		if (msg[0] == 'INIT-ACK'):
			self.mtype = MSG_TYPES[msg[0]]
			self.mlen = 2
			self.timestamp = get_curr_timestamp()
			self._buf = self.__pack_header(self.mtype, self.mlen)
			self._buf += chr((self.timestamp >> 8) & 0xff) + chr(self.timestamp & 0xff)
			return (self._buf)
		if (msg[0] in ['SET-STATE', 'STATE-ACK']):
			self.mtype = MSG_TYPES[msg[0]]
			self.states = msg[1]
			self.mlen = len(self.states)
			self._buf = self.__pack_header(self.mtype, self.mlen)
			self.__put_list_in_buf(self.states)
			return (self._buf)
		if (msg[0] == 'DATA'):
			self.mtype = MSG_TYPES[msg[0]]
			self.sid = msg[1] & 0xff
			self.timestamp = msg[2] & 0xffff
			self.data = msg[3]
			self.mlen = len(self.data) + 3
			self._buf = self.__pack_header(self.mtype, self.mlen)
			self._buf += self.__pack_header(self.sid, self.timestamp)
			self.__put_list_in_buf(self.data)
			return (self._buf)
		if (msg[0] in ['SET-TIMING', 'TIMING-ACK']):
			self.mtype = MSG_TYPES[msg[0]]
			self.t_range = msg[1]
			self.t_sample = msg[2]
			self.t_next = msg[3]
			self.t_cycle = msg[4]
			self.mlen = 5
			self._buf = self.__pack_header(self.mtype, self.mlen)
			self._buf += chr(self.t_range & 0xff) +  chr(self.t_sample & 0xff) + chr(self.t_next & 0xff)
			self._buf +=  chr((self.t_cycle >> 8) & 0xff) + chr(self.t_cycle & 0xff)
			return (self._buf)
		if (msg[0] == 'DATA'):
			self.mtype = MSG_TYPES[msg[0]]
			self.sid = msg[1] & 0xff
			self.timestamp = msg[2] & 0xffff
			self.data = msg[3]
			self.mlen = len(self.data) + 3
			self._buf = self.__pack_header(self.mtype, self.mlen)
			self._buf += self.__pack_header(self.sid, self.timestamp)
			self.__put_list_in_buf(self.data)
			return (self._buf)
		if (msg[0] == 'KEEPALIVE'):
			self.mtype = MSG_TYPES[msg[0]]
			if (msg[1]):
				self.timestamp = get_curr_timestamp()
				self.mlen = 2
				self._buf = self.__pack_header(self.mtype, self.mlen)
				self._buf += chr((self.timestamp >> 8) & 0xff) + chr(self.timestamp & 0xff)
			else:
				self.mlen = 0
				self._buf = self.__pack_header(self.mtype, self.mlen)
			return (self._buf)
			
	# Initialize the message for receiving
	# 	sockbuf should be a socket buffer, and have received at least 3 bytes (TYPE,LEN)
	#	function will returns additional bytes expected to receive (i.e. LEN field)
	#	calling function should add this return value to the sockbuff's elen value
	def recv_start(self, sockbuf):
		self._buf = sockbuf.buf[:3]
		(self.mtype, self.mlen) = self.__unpack_header(self._buf)
		return (self.mlen)
		
	# Receive the remaining messages
	#	sockbuf should be a socket buffer, and have all the rmeaining bytes as returned by recv_start()
	def recv_end(self, sockbuf):
		if (self.mlen > 0):
			self._buf += sockbuf.buf[3:]
			if ((self.mtype == MSG_TYPES['INIT-ACK']) or
			   (self.mtype == MSG_TYPES['KEEPALIVE'])):
				self.timestamp = (ord(self._buf[3]) << 8) + ord(self._buf[4])
			if ((self.mtype == MSG_TYPES['SET-STATE']) or
			   (self.mtype == MSG_TYPES['STATE-ACK'])):
				self.states = self.__recv_list_from_buf(self._buf[3:])
			if ((self.mtype == MSG_TYPES['SET-TIMING']) or
			   (self.mtype == MSG_TYPES['TIMING-ACK'])):
				self.t_range = ord(self._buf[3])
				self.t_sample = ord(self._buf[4])
				self.t_next = ord(self._buf[5])
				self.t_cycle = (ord(self._buf[6]) << 8) + ord(self._buf[7])
			if (self.mtype == MSG_TYPES['DATA']):
				self.sid = ord(self._buf[3])
				self.timestamp = (ord(self._buf[4]) << 8) + ord(self._buf[5])
				self.data = self.__recv_list_from_buf(self._buf[6:])
		
	# -- private methods
	def __put_list_in_buf(self, ilist):
		for a in ilist:
			self._buf += chr(a&0xff)
			
	def __recv_list_from_buf(self, buf):
		r = []
		for a in buf:
			r.append(ord(a))
		return (r)
		
	def __pack_header(self, byte, short):
		return chr(byte&0xff) + chr((short>>8)&0xff) + chr(short&0xff)

	def __unpack_header(self, buf):
		return ord(buf[0]), (ord(buf[1]) << 8) | ord(buf[2])
		
	# -- debug
	def debug(self, label=''):
		s = label + " : "
		t = "UNKNOWN"
		for tk in MSG_TYPES:
			if self.mtype == MSG_TYPES[tk]:
				t = tk
				break
		s += "T:[%s]-L:[%d]" % (t, self.mlen)
		if self.mlen > 0:
			s += "-P:0x"
			if self.mlen < 8:
				for i in xrange(self.mlen):
					x = ord(self._buf[3+i])
					s += "[%02x]" % x
			else:
				for i in [3,4,5]:
					x = ord(self._buf[i])
					s += "[%02x]" % x
				s += "..."
				for i in [3,2,1]:
					x = ord(self._buf[len(self._buf)-i])
					s += "[%02x]" % x
		print s

# Communications Socket base class
class CommsSock(object):
	def __init__(self):
		#self.ubyte = struct.Struct('B')
		#self.ushort = struct.Struct('>H')
		#self.umsg = struct.Struct('>BH')
		self.sbuf = SockBuf()
		self.rbuf = SockBuf()

	# Creates a new socket
	def new_sock(self):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		return (s)
		
	# send a message
	#	m is a tuple containing the message to be sent.  See CommsMsg.set_msg() for
	#	details on how to specify the message in tuple
	def send(self, m):
		msg = CommsMsg()
		self.sbuf.reset(buf=msg.set_msg(m))
		while (self.sbuf.clen < len(self.sbuf.buf)):
			r = self.sbuf.send(self.sock)
			if (r < 0):
				return (-1)	
		
	# send a CommsMsg
	#	msg should be a CommsMsg that is properly setup (eg. using CommsMsg.set_msg())
	def send_msg(self, msg):
		self.sbuf.reset(buf=msg._buf)
		while (self.sbuf.clen < len(self.sbuf.buf)):
			r = self.sbuf.send(self.sock)
			if (r < 0):
				return (-1)
	
	# receive a message
	#	Function returns a CommsMsg instance holding the received message
	def recv_msg(self):
		self.rbuf.reset(elen=3)
		if self.rbuf.recv(self.sock) < 0:
			return None
		msg = CommsMsg()
		self.rbuf.elen += msg.recv_start(self.rbuf)
		if self.rbuf.recv(self.sock) < 0:
			return None
		msg.recv_end(self.rbuf)
		return (msg)
		
# Communications Socket for Client (PC)
class CommsClient(CommsSock):
	# server should be a tuple specifying the IP and PORT
	def __init__(self, server):
		CommsSock.__init__(self)
		self.server = server
		
	# Connects to the server
	def connect(self):
		self.sock = self.new_sock()
		self.sock.connect(self.server)
		
	# send a message and receive its response
	# the message m is specified in tuple format (see CommsMsg.set_msg())
	# returns the received message in a CommsMsg form
	def send_and_recv(self, m):
		self.send(m)
		return (self.recv_msg())

	# close communications
	def close(self):
		self.sock.close()
		self.sock = None

# Communications Socket for Server (RPi2)
class CommsServer(CommsSock):
	# server should be a tuple specifying the IP and PORT
	def __init__(self, server):
		CommsSock.__init__(self)
		self.svr = self.new_sock()
		self.svr.bind(server)
		self.svr.listen(1)
		self._socks = [ self.svr ]
		self.stop = False

	# Wait for new connection or messages
	#	timeout is the number of seconds to wait
	#	If timeout < 0, this will be an infinite loop
	#	Depending on what causes the wait to be over, function returns two values: TYPE and PARAM
	#	If new connection, TYPE="CONN", PARAM=client's IP address & Port tuple
	#	If new message, TYPE="MSG", PARAM=the new message
	#	If timeout, TYPE="TIME", PARAM=None
	#	If self.stop is set to True, TYPE="STOP", PARAM=None
	def comms_wait (self, timeout=-1.0):
		tout = timeout
		if (timeout < 0):
			tout = 1.0
		while (not self.stop):
			ifs, ofs, efs = select.select (self._socks, [], [], tout)
			if self.svr in ifs:
				(self.sock, caddr) = self.svr.accept()
				self._socks.append(self.sock)
				return "CONN", caddr
			if len(ifs) > 0:
				self.sock = ifs[0]
				return "MSG", self.recv_msg()
			if (timeout > 0.0):
				return "TIME", None
		return "STOP", None
	
	# Close active communication socket
	def close_conn(self):
		if self.sock in self._socks:
			idx = self._socks.index(self.sock)
			self.sock.close()
			del self._socks[idx]
			
	# Shutdown server
	def shutdown(self):
		self.stop = True
		self.close_conn()
		self.svr.close()
			
if __name__ == "__main__":
	cs = CommsSock()
	msg = CommsMsg(cs, ('STATE-ACK',[1,0,0,1]))
	print repr(msg._buf)
	msg = CommsMsg(cs, ('KEEPALIVE',1))
	print repr(msg._buf)
	
