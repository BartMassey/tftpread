CFLAGS = -g -Wall

tftpread: tftpread.c
	$(CC) $(CFLAGS) -o tftpread tftpread.c

clean:
	-rm -f tftpread
