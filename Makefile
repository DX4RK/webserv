# INIT #

NAME = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I. -Iincludes

SRC_DIR = src
OBJ_DIR = obj

# Find all .cpp files in SRC_DIR and its subdirectories
SRCS = $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Replace SRC_DIR paths with OBJ_DIR paths for object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# COLORS #

BOLD=\e[1m
RESET=\e[0m

LIGHT_BLUE=\033[94m
LIGHT_CYAN=\033[96m
LIGHT_GREEN=\033[92m
LIGHT_YELLOW=\033[93m
LIGHT_ORANGE=\033[91m
LIGHT_PURPLE=\033[95m

# MAIN #

all: $(NAME)
	@printf "${LIGHT_BLUE}${BOLD}[$(NAME)]${RESET} built was successfull.\n"
	@echo "$(LIGHT_CYAN)$(BOLD)[information]$(RESET) startup: $(LIGHT_PURPLE)$(BOLD)./$(NAME)$(RESET)"

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "${LIGHT_YELLOW}${BOLD}[>] ${LIGHT_ORANGE}Building: %-33.33s\r${RESET}" $@
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.SILENT:
.PHONY: all clean fclean re
