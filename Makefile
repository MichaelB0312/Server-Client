# Makefile for the ttftps program
.PHONY: clean clean_all clean_zip all
all: ttftps
CC = g++
CFLAGS = -std=c++11 -Wall -g -pedantic -DNDEBUG -pthread
# CFLAGS = -std=c++11 -Wall -Werror -pedantic -pedantic-errors -DNDEBUG
# CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = ttftps.o connections.o
RM = rm -f
ZIP_FILENAME = 316124221_206013914.zip
LOG_FILENAME = log.txt
# Create the zip file for submission
zip:
	zip $(ZIP_FILENAME) *.h *.cpp Makefile README
# Creating the  executable, This is the Server 
ttftps: $(OBJS)
	$(CCLINK) -o ttftps $(OBJS)
# Creating the object files
ttftps.o: ttftps.cpp
	$(CC) -c $(CFLAGS) $< -o $@
connections.o: connections.cpp
	$(CC) -c $(CFLAGS) $< -o $@

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* ttftps
clean_zip:
	$(RM) $(ZIP_FILENAME)
clean_log:
	$(RM) $(LOG_FILENAME)
clean_all: clean clean_zip clean_log
