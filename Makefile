build: main.c application.c application.h dataProtocol.c dataProtocol.h setFD.c setFD.h commandMessages.c commandMessages.h physicalProtocol.c physicalProtocol.h transmitter.c transmitter.h receiver.c receiver.h byteStuffing.c byteStuffing.h options.c options.h dataProtection.c dataProtection.h
	gcc -g -o main.exe main.c dataProtocol.c setFD.c commandMessages.c physicalProtocol.c transmitter.c receiver.c byteStuffing.c options.c dataProtection.c
	gcc -g -o app.exe application.c dataProtocol.c setFD.c commandMessages.c physicalProtocol.c transmitter.c receiver.c byteStuffing.c options.c dataProtection.c

ports:
	sudo socat -d  -d  PTY,link=/dev/ttyS10,mode=777   PTY,link=/dev/ttyS11,mode=777

run: receiver transmitter

receiver:
	./main.exe 1
transmitter:
	./main.exe 2
