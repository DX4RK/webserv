# INIT #

NAME = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I. -Iincludes

SRC_DIR = src
OBJ_DIR = obj

# Find all .cpp files in SRC_DIR and its subdirectories
SRCS = $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Replace SRC_DIR paths with OBJ_DIR paths for object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# COLORS #

BOLD=\e[1m
RESET=\e[0m

LIGHT_CYAN=\033[96m
LIGHT_PURPLE=\033[95m

# MAIN #

all: $(NAME)
	@echo "$(BOLD)$(LIGHT_CYAN)$(NAME) is ready!$(RESET)"

$(NAME): $(OBJS)
	@echo "$(LIGHT_PURPLE)Building $(NAME)"
	@echo "$(RESET)"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Ensure OBJ_DIR mirrors SRC_DIR structure
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.SILENT:
.PHONY: all clean fclean re
