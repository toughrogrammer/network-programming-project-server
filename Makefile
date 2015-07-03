CC = gcc
CFLAGS = -g -std=c99
PATH_BUILD = ./build
PATH_PARSON = library/parson
OBJS = $(PATH_BUILD)/*.o
SRCS = main.c listening.c client_worker.c user.c game.c util.c MemLog.c $(PATH_PARSON)/parson.c

netp:
	@- echo "build::"
	$(CC) $(CFLAGS) -lm -o $(PATH_BUILD)/$@ $(SRCS)
	@- echo ""

clean:
	@- echo "cleaning::"
	rm -rf $(PATH_BUILD)/*
	@- echo ""

run:
	@- echo "result of run::"
	@- $(PATH_BUILD)/netp
	@- echo ""
