cc = gcc

TARGET = alfish

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(cc) -o $(TARGET) $(TARGET).c

clean:
	rm $(TARGET) 
