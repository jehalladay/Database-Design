# This make file will monitor changes to BTreeFreeLL.cpp
#    on change, it will delete the old executable and
#    recompile the program


run: BTreeFreeLL.o
	./BTreeFreeLL.o

runFL: BTreeFreeLL.o
	./BTreeFreeLL.o FL

runBT: BTreeFreeLL.o
	./BTreeFreeLL.o BT

BTreeFreeLL: BTreeFreeLL.cpp BTreeFreeLL.o
	g++ -W -o BTreeFreeLL.o BTreeFreeLL.cpp


clean: BTreeFreeLL.o
	rm BTreeFreeLL.o BTreeFreeLL.s

# in start we check to see if BTreeFreeLL.o exists, if so we delete it

start:
	@if [ -e BTreeFreeLL.o ]; then rm BTreeFreeLL.o; fi
	@make BTreeFreeLL
	@make run

testFL:
	@if [ -e BTreeFreeLL.o ]; then rm BTreeFreeLL.o; fi
	@make BTreeFreeLL
	@make runFL

testBT:
	@if [ -e BTreeFreeLL.o ]; then rm BTreeFreeLL.o; fi
	@make BTreeFreeLL
	@make runBT

# End of MakeFile

test:
	rm ./build/ex1.out ./intIndex.idx ./test.idx; g++ -Wall ./source/main.cpp -o ./build/ex1.out; ./build/ex1.out