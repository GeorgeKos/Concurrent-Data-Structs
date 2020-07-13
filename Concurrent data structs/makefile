build:
	@echo "Building main..."
	g++ -std=c++14 -Wall -Wextra -pedantic main.cpp concurrent_tree.cpp concurrent_queue.cpp concurrent_list.cpp -lpthread -o music.out
run:
	./music.out 4
clean:
	@echo "Cleaning executables.."
	rm *.out
	
ssh_kiwi:
	ssh kiwi.csd.uoc.gr