__author__ = 'caozj'

import socket
import threading
import sys
import os
import re
import binascii as bin
import random as ran

size = 8192
HOST = 'localhost'
PORT = 21
BUFSIZE = 4096
ADDR = (HOST, PORT)

def cur_dir():
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)


class port_thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.ip = ""
        self.port = 0
        self.store_or_return = False
        self.sock = None
        self.send_data = ""
        self.file_name = "data_log.txt"

    def run(self):
        (cli_sock, addr) = self.sock.accept()
        if not self.store_or_return:
            data_d = ""
            while True:
                temp_data = cli_sock.recv(BUFSIZE)
                data_d += temp_data;
                if not temp_data:
                    break
            with open(self.file_name, "wb") as file_out:
                    file_out.write(data_d)
        else:
            cli_sock.send(self.send_data)
        cli_sock.close()

    def setStoreData(self, data):
        self.send_data = data
        self.store_or_return = True

    def setFileName(self, name):
        self.file_name = name

    def setIp(self, ip_add):
        self.ip = str(ip_addr[0]) + '.' + str(ip_addr[1]) + '.' + str(ip_addr[2]) + '.' + str(ip_addr[3])
        self.port = int(ip_addr[4]) * 256 + int(ip_addr[5])
        self.store_or_return = False
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.send_data = ""
        self.sock.bind((self.ip, self.port))
        self.sock.listen(1)

    def __stop__(self):
        self.sock.close()

try:
    argv_length = len(sys.argv)
    for i in range(argv_length):
        if sys.argv[i] == '-port':
            PORT = int(sys.argv[i+1])
            ADDR = (HOST, PORT)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(ADDR)
    print(sock.recv(BUFSIZE))
    port_or_pasv = False
    has_port_or_pasv = False
    file_content = ""
    new_thread = None
    pasv_retr_filename = "data_log.txt"
    while True:
        data = raw_input("User:")
        data += '\r\n'
        if (data.find('RETR') == 0 or data.find('LIST') == 0 or data.find('NLST') == 0) and has_port_or_pasv:
            file_name = data[4:]
            file_name = file_name.strip()
            if port_or_pasv:
                if file_name:
                    pasv_retr_filename = file_name
                transfer_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                transfer_sock.connect((ip_server[0] + '.' + ip_server[1] + '.' + ip_server[2] + '.' + ip_server[3],
                                       int(ip_server[4]) * 256 + int(ip_server[5])))
            else:
                if file_name:
                    new_thread.setFileName(file_name)
                new_thread.start()
        elif data.find('PORT') == 0:
            if has_port_or_pasv:
                continue
            new_thread = port_thread()
            has_port_or_pasv = True
            data_buffer = data.strip()
            ip_addr = data_buffer[5:].split(',')
            port_or_pasv = False
            new_thread.setIp(ip_addr)
        elif data.find('PASV') == 0:
            if has_port_or_pasv:
                continue
            port_or_pasv = True
        elif data.find('QUIT') == 0 or data.find('ABOR') == 0:
            sock.send(data)
            print(sock.recv(BUFSIZE))
            break
        elif (data.find('STOR') == 0 or data.find('APPE') == 0) and has_port_or_pasv:
            file_name = data[4:]
            file_name = file_name.strip()
            with open(file_name, "rb") as file:
                file_content = file.read()
            if not port_or_pasv:
                new_thread.setStoreData(file_content)
                new_thread.start()
                

        sock.send(data)

        if (data.find('RETR') == 0 or data.find('LIST') == 0 or data.find('NLST') == 0) and port_or_pasv and has_port_or_pasv:
            result = sock.recv(BUFSIZE)
            result_list = result.split("\r\n")
            print(result_list[0])
            if result.find('150') == 0:
                with open(pasv_retr_filename, "wb") as file_f:
                    file_content = transfer_sock.recv(BUFSIZE)
                    transfer_sock.close()
                    file_f.write(file_content)
                if len(result_list) <= 2:
                    print(sock.recv(BUFSIZE))
                else:
                    print(result_list[1])
            has_port_or_pasv = False
        elif (data.find('RETR') == 0 or data.find('LIST') == 0 or data.find('NLST') == 0) and has_port_or_pasv:
            result = sock.recv(BUFSIZE)
            result_list = result.split("\r\n")
            print(result_list[0])
            if len(result_list) <= 2:
                print(sock.recv(BUFSIZE))
            else:
                print(result_list[1])
            has_port_or_pasv = False
        elif (data.find('STOR') == 0 or data.find('APPE') == 0) and port_or_pasv and has_port_or_pasv:
            transfer_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            transfer_sock.connect((str(ip_server[0]) + '.' + str(ip_server[1]) + '.' + str(ip_server[2]) + '.' + str(ip_server[3]),
                                   int(ip_server[4]) * 256 + int(ip_server[5])))
            while file_content:
                byte_num = transfer_sock.send(file_content)
                file_content = file_content[byte_num:]
            transfer_sock.close()
            file_content = ""
            result = sock.recv(BUFSIZE)
            result_list = result.split("\r\n")
            print(result_list[0])
            if len(result_list) <= 2:
                print(sock.recv(BUFSIZE))
            else:
                print(result_list[1])
            has_port_or_pasv = False
        elif (data.find('STOR') == 0 or data.find('APPE') == 0) and has_port_or_pasv:
            has_port_or_pasv = False
            file_content = ""
            result = sock.recv(BUFSIZE)
            result_list = result.split("\r\n")
            print(result_list[0])
            if len(result_list) <= 2:
                print(sock.recv(BUFSIZE))
            else:
                print(result_list[1])
        elif data.find('PASV') == 0:
            result = sock.recv(BUFSIZE)
            if not result:
                continue
            print(result)
            has_port_or_pasv = True
            port_or_pasv = True
            ip_server = re.search(r"(\d*,\d*,\d*,\d*,\d*,\d*)", result).group()
            ip_server = re.search(r"\d*,\d*,\d*,\d*,\d*,\d*", result).group()
            ip_server = ip_server.split(',')
        else:
            print(sock.recv(BUFSIZE))
    sock.close()

except Exception as reason:
    sock.close()
    print (reason)
