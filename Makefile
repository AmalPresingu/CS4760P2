CC = gcc
CFLAGS = -g -I -std = gnu99
OBJECTS1 = master.c config.h
OBJECTS2 = slave.c config.h
TARGET1 = master
TARGET2 = slave

$(TARGET1): $(OBJECTS1)
	$(CC) -o $@ $^ $(CFLAGS)

$(TARGET2): $(OBJECTS2)
	$(CC) -o $@ $^ $(CFLAGS)

clean: 
	rm master slave cstest logfile.*


