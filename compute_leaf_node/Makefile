CC = g++

# Uncomment one of the following to switch between debug and opt mode
OPT = -O3 -DNDEBUG
#OPT = -g -ggdb

CFLAGS += -Wall -c -std=c++11 -I. -I./include -I/usr/include/  $(OPT)

LDFLAGS+= -Wall -lssl -lcrypto -g -ggdb


LIBOBJECTS = \

HEADERS = *.h \

TEST = test
all: $(TEST)
#$(wildcard src/*.h)


clean:
	rm -f $(TEST) *.o

test: test.o $(LIBOBJECTS) 
	$(CC) test.o $(LIBOBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp  ${HEADERS} Makefile
	$(CC) $(CFLAGS) $< -o $@

#%.o: %.c  ${HEADERS} Makefile
#	$(CC) $(CFLAGS) $< -o $@

#./11-11_switch/%.o: ./11-11_switch/%.cpp  
#	$(CC) $(CFLAGS) $< -o $@
