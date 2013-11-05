all:
	gcc -D__USE_GNU -Wall nullfs.c printf.c log.c `pkg-config fuse --cflags --libs` -o nullfs

debug:
	gcc printf.c -o printf -DDEBUG -D__USE_GNU

clean:
	rm -f nullfs printf
