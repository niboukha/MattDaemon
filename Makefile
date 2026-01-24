NAME = Matt_daemon

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I./src

SRC_DIR = src
OBJ_DIR = obj

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/Daemon.cpp

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
