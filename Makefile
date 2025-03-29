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

clean:
	rm -f *.o all
