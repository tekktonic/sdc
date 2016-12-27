LDFLAGS = -lSDL2 -lSDL2_image
CFLAGS = -Wall -Wextra -std=c11 -g
OBJS = main.o

sdc1: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
clean:
	rm *.o *~ sdc1

