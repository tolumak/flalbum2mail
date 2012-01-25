OBJS:=main.o

APP:=flalbum2mail
CFLAGS:=-g $(shell pkg-config --cflags flickcurl)
LDFLAGS:=$(shell pkg-config --libs flickcurl)

all: $(APP)

$(APP): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc -c $(CFLAGS) -o $@ $^