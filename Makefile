CC = gcc

TARGET=rpitest

INCLUDEDIR = ./

INCDIR  = $(patsubst %,-I%,$(INCLUDEDIR))

OPT = -O0
DEBUG = -g
WARN= -Wall

CFLAGS= $(OPT) $(DEBUG) $(WARN) $(INCDIR)

INC = rpiPWM.h
SRC = rpiPWM.c rpitest.c

OBJ = $(SRC:.c=.o)

all: $(OBJ)
	$(CC)  $(OBJ) -o $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS)  -c $<  

clean:
	-rm *.o $(TARGET)
