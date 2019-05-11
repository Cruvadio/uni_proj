CC=g++

UNAME_S=$(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS = -lglut -lGL -lGLU -lm
endif
ifeq ($(UNAME_S), Darwin)
	CFLAGS = -framework OpenGl -framework GLUT -lm
endif

iteration1: iteration1.cpp
	$(CC) $^ -o $@ $(CFLAGS)

iteration2: iteration2.cpp
	$(CC) $^ -o $@ $(CFLAGS)

iteration3: iteration3.cpp
	$(CC) $^ -o $@ $(CFLAGS)

