#!/usr/bin/python

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

	# time.sleep(2)
	data = sock_client1.recv(MAX_LENGTH)
	print '(client1) response : %s' % data

	decoded = json.loads(data)
	access_token = decoded['access_token']


	sock_client1.send('%s\r\n' % (json.dumps({
		'target': 3, 
		'access_token': access_token,
		'title': 'hello world!'
	})))

	# time.sleep(2)
	while True:
		time.sleep(1)
		data = sock_client1.recv(MAX_LENGTH)
		if data:
			print '(client1) response : %s' % data


def client2():
	# waiting for creating game room
	time.sleep(2)

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

	# time.sleep(2)
	data = sock_client2.recv(MAX_LENGTH)
	print '(client2) response : %s' % data

	decoded = json.loads(data)
	access_token = decoded['access_token']


	sock_client2.send('%s\r\n' % (json.dumps({
		'target': 4, 
		'access_token': access_token,
		'room_id': 1
	})))

	# time.sleep(2)
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
	pass