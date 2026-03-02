NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
RM			= rm -f

SRCS_DIR	= srcs
OBJS_DIR	= objs
INCS_DIR	= includes

SRCS		= main.cpp \
			  Server.cpp \
			  Client.cpp \
			  Utils.cpp

OBJS		= $(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))

HEADERS		= $(INCS_DIR)/Server.hpp \
			  $(INCS_DIR)/Client.hpp \
			  $(INCS_DIR)/Utils.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCS_DIR) -c $< -o $@

clean:
	$(RM) -r $(OBJS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
