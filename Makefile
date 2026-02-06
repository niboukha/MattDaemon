NAME = Matt_daemon
GUI_NAME = Matt_daemon_gui

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -fsanitize=address -g

# Qt Configuration
QT_INCLUDE = -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtGui -I/usr/include/x86_64-linux-gnu/qt5/QtCore -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork
QT_LIBS = -lQt5Widgets -lQt5Gui -lQt5Core -lQt5Network
QT_FLAGS = -fPIC $(QT_INCLUDE)

OBJ_DIR = obj

SRCS = main.cpp \
       daemon/Daemon.cpp \
       network/Server.cpp \
       network/Client.cpp \
       util/Tintin_reporter.cpp\
	   parser/CommandParser.cpp \
	   CommandExecutor/CommandExecutor.cpp \
	   mailService/mailService.cpp 

GUI_SRCS = GuiMain.cpp \
           Gui/Gui.cpp \


OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJS = $(GUI_SRCS:%.cpp=$(OBJ_DIR)/gui_%.o)

all: $(NAME)

gui: $(GUI_NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(GUI_NAME): $(GUI_OBJS)
	$(CXX) $(CXXFLAGS) $(QT_FLAGS) $(GUI_OBJS) -o $(GUI_NAME) $(QT_LIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/gui_%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(QT_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(GUI_NAME)

re: fclean all

.PHONY: all gui clean fclean re
