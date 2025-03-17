# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
	pio -f run

upload:
	pio -f run --target upload

clean:
	pio -f run --target clean

program:
	pio -f run --target program

uploadfs:
	pio -f run --target uploadfs

update:
	pio -f update
