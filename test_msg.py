from socket import *
import sys
import json
import time
import thread


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


def client1():
	sock_client1 = socket(AF_INET, SOCK_STREAM)
	try:
		sock_client1.connect(ADDR)
	except Exception as e:
		print e
		sys.exit()

	print sock_client1


	sock_client1.send('%s\r\n' % (json.dumps({
		'target': 1, 
		'id': 'testuser2',
		'password': 'testuser2'
	})))
	data = sock_client1.recv(MAX_LENGTH)

	decoded = json.loads(data)
	access_token = decoded['access_token']

	while True:
		time.sleep(2)

		sock_client1.send('%s\r\n' % (json.dumps({
			'target': 2, 
			'access_token': access_token,
			'message': 'hello world!'
		})))
		print '(client1) chaaaaaaaat!'

		data = sock_client1.recv(MAX_LENGTH)
		if data:
			print '(client1) response : %s' % data


def client2():
	sock_client2 = socket(AF_INET, SOCK_STREAM)
	try:
		sock_client2.connect(ADDR)
	except Exception as e:
		print e
		sys.exit()

	print sock_client2

	sock_client2.send('%s\r\n' % (json.dumps({
		'target': 1, 
		'id': 'testuser3',
		'password': 'testuser3'
	})))

	while True:
		time.sleep(1)
		data = sock_client2.recv(MAX_LENGTH)
		if data:
			print '(client2) response : %s' % data


try:
	thread.start_new_thread(client1, ())
	thread.start_new_thread(client2, ())
except Exception as e:
	print 'exception : %s' % e
	pass


while 1:
	time.sleep(1)
	pass