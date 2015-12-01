
.PHONY: all
JC=javac
JFLAGS=-g

CURR_DIR=.
SRC_DIR=$(CURR_DIR)/src
BIN_DIR=$(CURR_DIR)/bin
MAIN_PACKAGE=homework_prima_implementazione
PACKAGES=$(MAIN_PACKAGE)

MAIN_FILES=$(SRC_DIR)/$(MAIN_PACKAGE)/*.java
TARGETS=$(BIN_DIR)/$(MAIN_PACKAGE)/MainClass.class

LATEX=$(CURR_DIR)/homework1.pdf

$(TARGETS): $(MAIN_FILES)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(JC) $(JFLAGS) -d $(BIN_DIR) -cp $(BIN_DIR) -sourcepath $(SRC_DIR) $(SRC_DIR)/$(MAIN_PACKAGE)/$(@F:.class=.java)

$(LATEX): $(LATEX:.pdf=.tex)
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make $(CURR_DIR)/$(@F:.pdf=.tex)

clean: clean-java clean-latex

clean-java:
	$(RM) $(BIN_DIR)/$(PACKAGES)/*.class

clean-latex:
	latexmk -C

java-targets: $(TARGETS)

latex-targets: $(LATEX)

all: java-targets latex-targets

