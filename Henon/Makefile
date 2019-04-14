CC=g++

UNAME_S=$(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS = -lglut -lGL -lGLU
endif
ifeq ($(UNAME_S), Darwin)
	CFLAGS = -framework OpenGl -framework GLUT
endif

Henon: main.cpp
	$(CC) $^ -o $@ $(CFLAGS)

