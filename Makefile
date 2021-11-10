build: main.c dataProtocol.c byteStuffing.c
	gcc -o dataLayer.exe main.c dataProtocol.c byteStuffing.c