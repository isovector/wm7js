CC      = gcc
CFLAGS  = -Wall -O -g
LIBS    = -lwiiuse
SRCS    = uinput.c  wm7js.c
OBJS    = $(SRCS:.c=.o)
MAIN    = wm7js


.PHONY: clean

all:		$(MAIN)

$(MAIN):	$(OBJS) 
		$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
		$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
		$(RM) *.o *~ $(MAIN)


# DO NOT DELETE THIS LINE -- make depend needs it
