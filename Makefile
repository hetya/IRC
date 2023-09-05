

# --> OS -------------------------------------------------------------------------
ifneq ($(OS), Windows_NT)
	OS_NAME = $(shell uname -s)
else
	OS_NAME = OS
endif

# --> PROGRAM --------------------------------------------------------------------
PROGRAM = ircserv
AUTHOR = -

# ~~~~~~~~~~~~~~~~ SOURCES ~~~~~~~~~~~~~~~~

SRCS_EXTENTION	=	.cpp
SRCS_PATH		=	src
SRCS            =   $(shell find $(SRCS_PATH) -name "*$(SRCS_EXTENTION)" -type f -print0 2>/dev/null | xargs -0 basename -a)

# ~~~~~~~~~~~~~~~~ OBJECTS ~~~~~~~~~~~~~~~~

OBJS_PATH		=	objs
OBJS			=	$(addprefix $(OBJS_PATH)/, $(SRCS:$(SRCS_EXTENTION)=.o))
# ~~~~~~~~~~~~~~~~ INCLUDES ~~~~~~~~~~~~~~~~

INCLUDES_EXTENTION  =   .hpp \
						.tpp
INCLUDES_PATH = includes
# find includes \( -name "*.hpp" -o -name "*.tpp" \) -type f -print0 2>/dev/null | xargs -0 basename -a
INCLUDES			=   $(shell find $(INCLUDES_PATH) -name "*$(wildcard$(INCLUDES_EXTENTION))" -type f -print0 2>/dev/null | xargs -0 basename -a)
ifeq ($(INCLUDES),)
	INCLUDES_FILES	=	""
else
	INCLUDES_FILES	=   $(addprefix $(INCLUDES_PATH)/, $(INCLUDES))
endif




# --> COMPILER AND FLAGS ---------------------------------------------------------

CC				= c++

FLAGS			= -std=c++98 -I./$(INCLUDES_PATH)
ifeq ($(OS_NAME), Linux)
	FLAGS			+= -I/usr/include -L /usr/lib -L/usr/local/lib -I/usr/local/include
endif
FLAGS			+= -Wall -Wextra -Werror -Wshadow -Wno-shadow
#  FLAGS			+= -fsanitize=address -g3
FLAGS			+= -g3


# --> COLOR ----------------------------------------------------------------------

ifneq (,$(findstring 256color, ${TERM}))

# ~~~~~~~~~~~~~~~~~ COLORS ~~~~~~~~~~~~~~~~~

	BLACK		:= $(shell tput -Txterm setaf 0)
	RED			:= $(shell tput -Txterm setaf 1)
	GREEN		:= $(shell tput -Txterm setaf 2)
	YELLOW		:= $(shell tput -Txterm setaf 3)
	BLUE		:= $(shell tput -Txterm setaf 4)
	PURPLE		:= $(shell tput -Txterm setaf 5)
	CYAN		:= $(shell tput -Txterm setaf 6)
	WHITE		:= $(shell tput -Txterm setaf 7)
	END			:= $(shell tput -Txterm sgr0)
	UNDER		:= $(shell tput -Txterm smul)
 	BOLD		:= $(shell tput -Txterm bold)
	rev			:= $(shell tput -Txterm rev)

# 	# ~~~~~~~~~~~~ BACKGROUND COLORS ~~~~~~~~~~~~

	IBLACK		:= $(shell tput -Txterm setab 0)
	IRED		:= $(shell tput -Txterm setab 1)
	IGREEN		:= $(shell tput -Txterm setab 2)
	IYELLOW		:= $(shell tput -Txterm setab 3)
	IBLUE		:= $(shell tput -Txterm setab 4)
	IPURPLE		:= $(shell tput -Txterm setab 5)
	ICYAN		:= $(shell tput -Txterm setab 6)
	IWHITE		:= $(shell tput -Txterm setab 7)

else
	BLACK		:= ""
	RED			:= ""
	GREEN		:= ""
	YELLOW		:= ""
	LIGHTPURPLE	:= ""
	PURPLE		:= ""
	BLUE		:= ""
	WHITE		:= ""
	END			:= ""
endif
# # --> TEST MODULE ----------------------------------------------------------------

# # --> RULES ----------------------------------------------------------------------

all:   header $(PROGRAM)

# ~~~~~~~~~~~~ COMPILING IN .o ~~~~~~~~~~~~

$(OBJS_PATH)/%.o:	$(SRCS_PATH)/%$(SRCS_EXTENTION) $(wildcard $(INCLUDES_FILES)) Makefile
	mkdir -p $(dir $@);
	printf "%-62b%b" "$(CYAN)$(BOLD)compiling $(END)$<"
	 ${CC} ${FLAGS} -c $< -o $@							
	printf "$(GREEN)[✓]$(END)\n"

# ~~~~~~~ COMPILING THE EXECUTABLE ~~~~~~~~

$(PROGRAM):	$(OBJS)
	printf "%-63b%b" "\n$(BOLD)$(GREEN)creating$(END) $@"
	$(CC) $(FLAGS) $(OBJS) -o $(PROGRAM)
	printf "$(GREEN)[✓]$(END)\n\n\n"

bot: header
	printf "%-63b%b" "\n$(BOLD)$(GREEN)creating$(END) $@"
	$(CC) $(FLAGS) bonus/src/bot.cpp -o bot
	printf "$(GREEN)[✓]$(END)\n\n\n"

# ~~~~~~~~~~~~ CLEANNING RULES ~~~~~~~~~~~~

clean:
	rm -rf $(OBJS_PATH)

fclean: clean
	rm -f $(PROGRAM)
	rm -f bot

# ~~~~~~~~~~~~~~ REMAKE RULE ~~~~~~~~~~~~~~
re: fclean all

# --> HEADER ---------------------------------------------------------------------

header :
	@printf " ________ ________      ______ _______   ______  \n" 
	@printf "|        \        \    |      \       \ /      \ \n" 
	@printf "| ▓▓▓▓▓▓▓▓\▓▓▓▓▓▓▓▓     \▓▓▓▓▓▓ ▓▓▓▓▓▓▓\  ▓▓▓▓▓▓\\n" 
	@printf "| ▓▓__      | ▓▓         | ▓▓ | ▓▓__| ▓▓ ▓▓   \▓▓\n" 
	@printf "| ▓▓  \     | ▓▓         | ▓▓ | ▓▓    ▓▓ ▓▓      \n" 
	@printf "| ▓▓▓▓▓     | ▓▓         | ▓▓ | ▓▓▓▓▓▓▓\ ▓▓   __ \n" 
	@printf "| ▓▓        | ▓▓        _| ▓▓_| ▓▓  | ▓▓ ▓▓__/  \\n" 
	@printf "| ▓▓        | ▓▓       |   ▓▓ \ ▓▓  | ▓▓\▓▓    ▓▓\n" 
	@printf " \▓▓         \▓▓        \▓▓▓▓▓▓\▓▓   \▓▓ \▓▓▓▓▓▓ \n" 
	echo
	echo "Author :" $(AUTHOR)
ifeq ($(OS_NAME), Linux)
	echo "Last modification :" `ls --time-style=long-iso  -la1rt | awk '{print $$6, $$7, $$9, $$8}' | tail -n 1`
else
	echo "Last modification :" `ls -T -la1rt | tail -n 1`
endif
	@printf "\n\n"


exec : all
	printf "$(BOLD)$(YELLOW)starting program : $(END)$(PROGRAM)\n\n\n"
	./$(PROGRAM)

show :
	printf "Program name : %s\n"		$(PROGRAM)
	printf "Author : %s\n"				$(AUTHOR)
	printf "Compilator : %s\n"			$(CC)
	printf "Flags %s\n"					$(FLAGS)
	printf "Source path : %s\n"			$(SRCS_PATH)
	printf "Sources : %s\n"				$(SRCS)
	printf "Object path : %s\n"			$(OBJS_PATH)
	printf "Objects : %s\n"				$(OBJS)
	printf "Include path : %s\n"		$(INCLUDES_PATH)
	printf "Includes : %s\n"			$(INCLUDES)
	printf "Includes with their : %s\n"	$(INCLUDES_FILES)
ifneq (,$(PROGRAM))
	printf "Color : $(GREEN)active$(END)\n"
else
		printf "Color : disable\n"
endif

debug : show

help :
	echo "Generic Makefile for my C/C++ Projects\n"
	echo "Usage : make [arg]\n"
	echo "Argument :\n"
	echo "	all			(=make) compile and link"
	echo "	objs		compile only (no linking)"
	echo "	clean		remove objects and dependencies"
	echo "	fclean		remove objects, dependencies and the executable"
	echo "	show		show variables (for debug use only)"
	echo "	help		printing help message"

.PHONY: all clean fclean re header exec show
.SILENT:
