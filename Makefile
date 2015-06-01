CC = gcc
CFLAGS = -g
PATH_BUILD = ./build
PATH_PARSON = library/parson
OBJS = $(PATH_BUILD)/*.o

main: main.o listening.o parson.o
	@- echo "build::"
	$(CC) $(CFLAGS) -lm -o $(PATH_BUILD)/$@ $(OBJS)
	@- echo ""

clean:
	@- echo "cleaning::"
	rm -rf $(PATH_BUILD)/*
	@- echo ""

run:
	@- echo "result of run::"
	@- $(PATH_BUILD)/main
	@- echo ""

main.o: 
	$(CC) $(CFLAGS) -c -o $(PATH_BUILD)/$@ main.c
listening.o:
	$(CC) $(CFLAGS) -c -o $(PATH_BUILD)/$@ listening.c
parson.o: 
	$(CC) $(CFLAGS) -c -o $(PATH_BUILD)/$@ $(PATH_PARSON)/parson.c