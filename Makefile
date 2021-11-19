CSOURCE=$(wildcard *.c)
CTARGETS=$(patsubst %.c, %.o, $(CSOURCE))
CXXSOURCE=$(wildcard *.cc)
CXXTARGETS=$(patsubst %.cc, %.o, $(CXXSOURCE))

CC=gcc
CXX=g++
CFLAGS=-Wall -O2
CXXFLAGS=-Wall -O2
LDFLAGS=-s -lpthread

# CC=gcc
# CXX=g++
# CFLAGS=-Wall -O2 -g -fsanitize=address
# CXXFLAGS=-Wall -O2 -g -fsanitize=address
# LDFLAGS=-lpthread
# CC=clang
# CXX=clang++
# CFLAGS=-Wall -g -fsanitize=address -O2
# CXXFLAGS=-Wall -g -fsanitize=address -O2
# LDFLAGS=-lpthread


server:$(CTARGETS) $(CXXTARGETS)
	$(CXX) $(CFLAGS) $(CTARGETS) $(CXXTARGETS) -o server $(LDFLAGS)

$(CTARGETS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(CXXTARGETS):%.o:%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(CTARGETS) $(CXXTARGETS) server
