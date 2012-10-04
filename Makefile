OBJS:=main.o

APP:=flalbum2mail
CFLAGS:=-g $(shell pkg-config --cflags flickcurl) -Wall
LDFLAGS:=$(shell pkg-config --libs flickcurl)

all: $(APP)

clean:
	rm -f *.o $(APP)

$(APP): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc -c $(CFLAGS) -o $@ $^