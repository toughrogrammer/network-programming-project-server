OBJS = main.o parson.o
PATH_PARSON = library/parson
SRC_MAIN = main.c

main: main.o parson.o
	@- echo "build::"
	gcc -lm -o ./build/$@ $(OBJS)
	@- echo ""

.c.o:
	gcc -g -c $<
parson.o: $(PATH_PARSON)/*
main.o: $(SRC_MAIN)

clean:
	@- echo "cleaning::"
	rm -rf ./build/*
	@- echo ""

run:
	@- echo "result of run::"
	@- ./build/main
	@- echo ""