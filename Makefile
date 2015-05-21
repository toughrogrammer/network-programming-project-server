OBJS = main.o parson.o
PATH_BUILD = ./build
PATH_PARSON = library/parson
SRC_MAIN = main.c

main: main.o parson.o
	@- echo "build::"
	gcc -lm -o $(PATH_BUILD)/$@ $(OBJS)
	@- echo ""

.c.o:
	gcc -g -c $<
parson.o: $(PATH_PARSON)/*
main.o: $(SRC_MAIN)

clean:
	@- echo "cleaning::"
	rm -rf $(PATH_BUILD)/*
	@- echo ""

run:
	@- echo "result of run::"
	@- $(PATH_BUILD)/main
	@- echo ""