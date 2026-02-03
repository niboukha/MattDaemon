NAME = Matt_daemon

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11

OBJ_DIR = obj

SRCS = main.cpp \
       daemon/Daemon.cpp \
       network/Server.cpp \
       network/Client.cpp \
       util/Tintin_reporter.cpp\
	   parser/CommandParser.cpp

OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
