main:Main.cpp
	g++ -std=c++17 -lspdlog $^ -o $@

.PHONY:debug
debug:
	g++ -std=c++17 Main.cpp -g -o debug

.PHONY:clean
clean:
	rm -rf main
