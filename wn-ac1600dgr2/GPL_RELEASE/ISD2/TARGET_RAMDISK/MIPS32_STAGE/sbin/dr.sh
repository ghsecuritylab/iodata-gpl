#!/bin/sh
rm -rf  $1
tftp -g -r $1 192.168.99.8
chmod 777 $1
./$1

