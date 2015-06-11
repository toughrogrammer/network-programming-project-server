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

def print_response(prefix, data):
	data = data.replace('\r\n', '\n')
	rows = data.split('\n')
	for row in rows:
		if not row:
			continue

		print '%s response :' % prefix

		parsed = json.loads(row)
		print json.dumps(parsed, indent=4, sort_keys=True)


sock_client = socket(AF_INET, SOCK_STREAM)
try:
	sock_client.connect(ADDR)
except Exception as e:
	print e
	sys.exit()

print sock_client


sock_client.send('%s\r\n' % (json.dumps({
	'target': 0, 
	'id': 'testuser10',
	'password': 'testuser10',
	'character_type': 1
})))

data = sock_client.recv(MAX_LENGTH)
print_response('client', data)

sock_client.send('%s\r\n' % (json.dumps({
	'target': 1, 
	'id': 'testuser10',
	'password': 'testuser10'
})))

data = sock_client.recv(MAX_LENGTH)
print_response('client', data)

decoded = json.loads(data)
access_token = decoded['access_token']
if access_token:
	print 'signin success : %s' % access_token