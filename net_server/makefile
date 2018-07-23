TARGET = netfileserver
OBJECTS = libnetfiles_server.o netfileserver.o
FLAGS = -Wall -fsanitize=address -g -pthread

$(TARGET): libnetfiles_server.o netfileserver.o
		gcc $(FLAGS) -o $@ $^


clean:
	rm -f $(TARGET) $(OBJECTS)

%.o: %.c
	gcc $(FLAGS) -c $<

libnetfiles_server.o: libnetfiles_server.h
 netfileserver.o:  libnetfiles_server.h
