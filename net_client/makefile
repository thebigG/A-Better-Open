TARGET = client
OBJECTS = libnetfiles.o Client.o
FLAGS = -Wall -fsanitize=address -g

$(TARGET): libnetfiles.o Client.o
		gcc $(FLAGS) -o $@ $^


clean:
	rm -f $(TARGET) $(OBJECTS)

%.o: %.c
	gcc $(FLAGS) -c $<

libnetfiles.o: libnetfiles.h
 Client.o:  libnetfiles.h
