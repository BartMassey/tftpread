# Copyright © 2012 Bart Massey
# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.

# [This program is licensed under the "MIT License"]
# Please see the file COPYING in the source
# distribution of this software for license terms.

CFLAGS = -g -Wall

tftpread: tftpread.c
	$(CC) $(CFLAGS) -o tftpread tftpread.c

clean:
	-rm -f tftpread
