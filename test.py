from socket import *
import sys
import json
import time


HOST = '127.0.0.1'
PORT = 10101
MAX_LENGTH = 4096
ADDR = (HOST, PORT)

def until_crlf(line):
	return line.split('\r')[0]

sock_client = socket(AF_INET, SOCK_STREAM)
try:
	sock_client.connect(ADDR)
except Exception as e:
	print e
	sys.exit()


sock_client.send('%s\r\n%s\r\n' % (json.dumps({
	'target': 1, 
	'id': 'testuser2',
	'password': 'testuser2'
}),
json.dumps({
	'target': 9, 
	'access_token': 'user0'
})))

time.sleep(1)
data = sock_client.recv(MAX_LENGTH)
print 'response : %s' % data