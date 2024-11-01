main:Main.cpp
	g++ $^ -o $@

.PHONY:debug
debug:
	g++ Main.cpp -g -o debug

.PHONY:clean
clean:
	rm -rf main
