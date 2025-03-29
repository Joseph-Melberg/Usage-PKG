objects = main.o utils.o
CC = gcc
CFLAGS = -static  -lrabbitmq 
#-static

.PHONY: clean testit
usage: $(objects)
	$(CC)  $^ $(CFLAGS) -o usage


main.o : main.c utils.h
	$(CC) -c $(CFLAGS) $^  $@

$(objects): %.o: %.c
	$(CC) -c  $^ $(CFLAGS)

%.c:
	touch $@

testit:
	./all rabbit.centurionx.net 5672 lifesecure pW1c8Qq7xdMqhZ7q

clean:
	rm -f *.o all
