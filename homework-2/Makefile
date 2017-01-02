CC = gcc
RM = rm

LIB_MK_PATH = ../libraries
include $(LIB_MK_PATH)/mclabutils.mk $(LIB_MK_PATH)/mlutils.mk $(LIB_MK_PATH)/mlclut.mk

# directories

CURR_DIR = .
SRC_DIR = $(CURR_DIR)
OBJ_DIR = $(CURR_DIR)
BIN_DIR = $(CURR_DIR)

SRCS = $(shell ls *.c)
OBJS = $(SRCS:.c=.o)
BINS = $(BIN_DIR)/homework_local \
	   $(BIN_DIR)/homework_cpu \
	   $(BIN_DIR)/homework_global \
	   $(BIN_DIR)/homework_unlimited

LATEX=$(CURR_DIR)/relazione.pdf

# headers and libraries

HDR_DIR = $(CURR_DIR)/include
INCLUDES = -I. -I$(HDR_DIR)

INCLUDES += -I$(MCLAB_INCLUDE) -I$(ML_INCLUDE) -I$(MLCLUT_INCLUDE)
LIBRARIES += -L$(MCLAB_LIB) -L$(ML_LIB) -L$(MLCLUT_LIB)

# compiler flags

CFLAGS_PRODUCTION = -O2 -DNDEBUG
CFLAGS = -g -fno-builtin --std=c99 --pedantic --pedantic-errors -Wall -Wextra -Wno-unused $(INCLUDES)

#CFLAGS += $(CFLAGS_PRODUCTION)

UNAME = $(shell uname)

#ifeq ($(UNAME), Darwin)
#BIN_FLAGS = -framework OpenCL
#else
#BIN_FLAGS = -lOpenCL
#endif

#CLFAGS += $(CFLAGS_PRODUCTION)

all: $(BINS) #$(LATEX)

$(LATEX): $(LATEX:.pdf=.tex)
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make $(CURR_DIR)/$(@F:.pdf=.tex)

$(BINS): $(OBJS)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(BIN_FLAGS) $(OBJ_DIR)/homework_utils.o $(OBJ_DIR)/$(@F).o $(LIBRARIES) -lmlclut -lmlutils -lMCLabUtils -lOpenCL -lm -o $@

$(OBJS): $(SRCS)
	test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/$(@F:.o=.c) -c -o $@

clean:
	$(RM) -rf $(OBJS)
	$(RM) -rf $(BINS)
	$(RM) -rf output.png

clean-latex:
	latexmk -C
