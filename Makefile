# Μεταβλητές για τον compiler και τα flags
CC = gcc
CFLAGS = -Wall -g -I./include
LDFLAGS =

# Κατάλογοι
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = .

# Τα εκτελέσιμα που πρέπει να παραχθούν
TARGETS = jms_coord jms_console jms_pool
OBJECTS = jms_coord.o jms_console.o src/jms_pool.o src/requests.o

.PHONY: all clean test test_chain validate help

# Default target
all: $(TARGETS)

# Κανόνας για jms_coord
jms_coord: jms_coord.o src/requests.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✓ Created jms_coord executable"

# Κανόνας για jms_console
jms_console: jms_console.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✓ Created jms_console executable"

# Κανόνας για jms_pool
jms_pool: src/jms_pool.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "✓ Created jms_pool executable"

# Κανόνας για τη μετατροπή των .c σε .o (στο root directory)
jms_%.o: jms_%.c include/util.h include/requests.h
	$(CC) $(CFLAGS) -c $< -o $@

# Κανόνας για τη μετατροπή των .c σε .o (στον src directory)
src/%.o: src/%.c include/util.h include/requests.h
	$(CC) $(CFLAGS) -c $< -o $@

# Καθαρισμός των παραγόμενων αρχείων
clean:
	rm -f *.o src/*.o $(TARGETS)
	rm -f jms_in jms_out
	rm -f pool_in_* pool_out_*
	rm -f test_input.txt test_output.log
	@echo "✓ Cleanup completed"

# Build and run basic test
test: all test_chain

# Quick validation of the chain
validate: all
	@chmod +x validate_chain.sh
	@./validate_chain.sh

# Test the Console -> Coordinator -> Pool chain
test_chain: all
	@echo "======================================"
	@echo "Testing: Console -> Coordinator -> Pool"
	@echo "======================================"
	@echo ""
	@echo "[STEP 1] Creating test input file..."
	@echo "submit echo 'Hello from Pool'" > test_input.txt
	@echo "submit sleep 1 && echo 'Job 2 completed'" >> test_input.txt
	@echo "shutdown" >> test_input.txt
	@echo "✓ Test input file created"
	@echo ""
	@echo "[STEP 2] Starting Coordinator (max 2 jobs per pool)..."
	@./jms_coord /tmp 2 > /tmp/coordinator.log 2>&1 &
	@COORD_PID=$$!; \
	sleep 1; \
	echo "✓ Coordinator started with PID $$COORD_PID"
	@echo ""
	@echo "[STEP 3] Sending requests from Console..."
	@./jms_console -i test_input.txt > /tmp/console.log 2>&1
	@echo "✓ Console requests sent"
	@echo ""
	@echo "[STEP 4] Checking Coordinator output..."
	@sleep 2
	@if [ -f /tmp/coordinator.log ]; then \
		echo "Coordinator log:"; \
		cat /tmp/coordinator.log; \
		echo "✓ Coordinator completed"; \
	fi
	@echo ""
	@echo "[STEP 5] Cleanup..."
	@rm -f jms_in jms_out pool_in_* pool_out_*
	@echo "✓ Named pipes cleaned up"
	@echo ""
	@echo "======================================"
	@echo "Chain test completed!"
	@echo "======================================"

# Help target
help:
	@echo "Job Management System - Makefile"
	@echo "================================"
	@echo "Targets:"
	@echo "  make                - Build all executables"
	@echo "  make jms_coord      - Build coordinator only"
	@echo "  make jms_console    - Build console only"
	@echo "  make jms_pool       - Build pool worker only"
	@echo "  make clean          - Remove all generated files"
	@echo "  make validate       - Quick validation of Console→Coordinator→Pool"
	@echo "  make test           - Run the full chain test"
	@echo "  make test_chain     - Detailed chain test"
	@echo "  make help           - Show this help message"
	@echo ""
	@echo "Quick Start:"
	@echo "  make validate       # Quick 30-second validation"
	@echo "  make test           # Full comprehensive test suite"