CC=gcc
RM=rm
CFLAGS=-g  -pthread -O2  -Wall -Werror -Wno-unused
BIN=udp_receiver
OBJS = main.o

$(BIN): $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) -rf $(OBJS) $(BIN)

