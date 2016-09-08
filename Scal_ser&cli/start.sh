#!/bin/sh

./client 192.168.1.9 12345 4 &
echo $! > pid_of_cli
