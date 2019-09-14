#!/bin/bash
openssl req -newkey rsa:4096 -nodes -sha512 -x509 -days 3650 -nodes -out ssl.pem -keyout ssl.key
openssl x509 -outform der -in ssl.pem -out ssl.der
openssl req -newkey rsa:4096 -nodes -sha512 -x509 -days 3650 -nodes -keyout server.key
