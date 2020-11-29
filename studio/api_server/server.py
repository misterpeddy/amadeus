#!/usr/bin/env python3

import logging
import os
import parse
import socket

HOST = '127.0.0.1'  
PORT = 8080 
BLOCK_SIZE = 1024

PAYLOAD_HEADER = b'\x00\x00\x00\x00D\x00\x00\x00'

PAYLOAD_SECTION_COMMAND = '<:COMMAND:>'
PAYLOAD_SECTION_FILEPATH = '<:FILEPATH:>'

COMMAND_TYPE_PRACTICE = 'PRACTICE'
COMMAND_TYPE_PLAY = 'PLAY'

RESPONSE_ERR = b'RESPONSE_ERR'
RESPONSE_AOK = b'RESPONSE_AOK'

logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))

def start():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        while True:
            conn, addr = s.accept()
            with conn:
                logging.info('Connected by ' + str(addr))
                data = conn.recv(BLOCK_SIZE)
                response = handle_request(data)
                conn.sendall(PAYLOAD_HEADER + response)


def handle_request(data):
    logging.info("Received request with payload: `{}`".format(str(data)))
    parsed = parse_request(data)
    if not parsed:
        return RESPONSE_ERR
    if parsed['command'] == COMMAND_TYPE_PRACTICE:
        logging.info("practice: " + parsed['path'])
        return RESPONSE_AOK
    return RESPONSE_ERR
        

def parse_request(data):
    if not data:
        return None
    headerPos = data.find(PAYLOAD_HEADER)
    if headerPos < 0:
        logging.error("Request payload `{}` did not contain header `{}`".format(data, PAYLOAD_HEADER))
        return None
    message = data[headerPos + len(PAYLOAD_HEADER) : ]
    message = message.decode('UTF-8')
    format = "{c}{{command}}{p}{{path}}".format(c=PAYLOAD_SECTION_COMMAND, p= PAYLOAD_SECTION_FILEPATH)
    parsed = parse.parse(format, message)
    if parsed == None or 'command' not in parsed or 'path' not in parsed:
        logging.error("Request can't be parsed correctly: " + str(parsed))
        return None
    return parsed    


if __name__ == '__main__':
    start()