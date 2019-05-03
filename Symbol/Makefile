CC=g++

UNAME_S=$(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS=-lglut -lGL -lGLU -g -lm
endif
ifeq ($(UNAME_S), Darwin)
	CFLAGS=-framework OpenGl -framework GLUT -g -lm
endif

symbol: symbol.cpp
	$(CC) $^ -o $@ $(CFLAGS)
