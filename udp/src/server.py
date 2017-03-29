__author__ = 'caozj'

import socket

size = 8192

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

try:
    i = 0
    while True:
        data, address = sock.recvfrom(size)
        data = str(i) + " " + data
        i += 1
        sock.sendto(data, address)
finally:
    sock.close()
