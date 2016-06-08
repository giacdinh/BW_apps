
all: hashfile.host hashfile

hashfile.host: hashfile.c Flash2AppSM_hash.c
	gcc -o hashfile.host -DHOST_BUILD=1 Flash2AppSM_hash.c hashfile.c -lssl -lcrypto

hashfile: hashfile.c Flash2AppSM_hash.c
	arm-poky-linux-gnueabi-gcc -o hashfile Flash2AppSM_hash.c hashfile.c -lssl -lcrypto
clean:
	rm -f *.o hashfile hashgenerate hashfile.host
