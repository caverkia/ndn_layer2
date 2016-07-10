CC = g++

# Uncomment one of the following to switch between debug and opt mode
OPT = -O3 -DNDEBUG
#OPT = -g -ggdb

CFLAGS += -Wall -c -std=c++11 -I. -I./include -I/usr/include/  $(OPT)

LDFLAGS+= -Wall -lssl -lcrypto -g -ggdb


LIBOBJECTS = \

HEADERS = *.h \

TEST = generate_stp
all: $(TEST)
#$(wildcard src/*.h)


clean:
	rm -f $(TEST) *.o

test: generate_stp.o $(LIBOBJECTS) 
	$(CC) generate_stp.o $(LIBOBJECTS) $(LDFLAGS) -o $@

%.o: %.cc  ${HEADERS} Makefile
	$(CC) $(CFLAGS) $< -o $@

#%.o: %.c  ${HEADERS} Makefile
#	$(CC) $(CFLAGS) $< -o $@

#./11-11_switch/%.o: ./11-11_switch/%.cpp  
#	$(CC) $(CFLAGS) $< -o $@
