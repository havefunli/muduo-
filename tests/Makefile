# 定义源文件和目标文件的目录
BIN_DIR = ../build/bin/
OBJ_DIR = ../build/obj/
SRC_DIR = ../src/

# 定义目标文件名和源文件
TARGET = $(BIN_DIR)test
DEBUG  = $(BIN_DIR)debug

# .o 文件
OBJS = $(OBJ_DIR)test-eventloop-timerwheel.o \
       $(OBJ_DIR)EventLoop.o \
       $(OBJ_DIR)Epoll.o \
       $(OBJ_DIR)Channel.o \
       $(OBJ_DIR)TimerWheel.o

# 编译器选项
CXXFLAGS = -std=c++17

# 链接器选项
LDFLAGS  = -lrt

# 默认目标
$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

.PHONY: debug
debug: $(OBJS)
	g++ -g $(OBJS) -o $(DEBUG) $(CXXFLAGS) $(LDFLAGS)

$(OBJ_DIR)test-eventloop-timerwheel.o: ./test-eventloop-timerwheel.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)EventLoop.o: $(SRC_DIR)EventLoop.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)Epoll.o : $(SRC_DIR)Epoll.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)Channel.o: $(SRC_DIR)Channel.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR)TimerWheel.o: $(SRC_DIR)TimerWheel.cpp
	g++ -c $< -o $@ $(CXXFLAGS)

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)




