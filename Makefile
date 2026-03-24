# Μεταβλητές για τον compiler και τα flags
CC = gcc
CFLAGS = -Wall -g
# Τα ονόματα των εκτελέσιμων που ζητάει η άσκηση
TARGET1 = jms_coord
TARGET2 = jms_console

# Όλα τα εκτελέσιμα που πρέπει να παραχθούν
all: $(TARGET1) $(TARGET2)

# Κανόνας για το jms_coord
$(TARGET1): jms_coord.o
	$(CC) $(CFLAGS) -o $(TARGET1) jms_coord.o

# Κανόνας για το jms_console
$(TARGET2): jms_console.o
	$(CC) $(CFLAGS) -o $(TARGET2) jms_console.o

# Κανόνας για τη μετατροπή των .c σε .o
%.o: %.c util.h
	$(CC) $(CFLAGS) -c $<

# Καθαρισμός των παραγόμενων αρχείων
clean:
	rm -f *.o $(TARGET1) $(TARGET2) jms_in jms_out
run_script:
	chmod +x jms_script.sh
	./jms_script.sh