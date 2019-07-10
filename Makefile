OBJS   = aptiofile.o main.o
TARGET = aptio
CFLAGS = -Wall -Wextra -ggdb -Og

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
