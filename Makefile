
.PHONY: all

JC=javac
JFLAGS=-g

CURR_DIR=.
SRC_DIR=$(CURR_DIR)/src
BIN_DIR=$(CURR_DIR)/bin

FIRST_PACKAGE=homework_prima_implementazione
FIRST_FILES=$(SRC_DIR)/$(FIRST_PACKAGE)/*.java
FIRST_TARGET=$(BIN_DIR)/$(FIRST_PACKAGE)/MainClass.class

SECOND_PACKAGE=homework_seconda_implementazione
SECOND_FILES=$(SRC_DIR)/$(SECOND_PACKAGE)/*.java
SECOND_TARGET=$(BIN_DIR)/$(SECOND_PACKAGE)/MainClass.class

LATEX=$(CURR_DIR)/homework1.pdf

all: java-targets latex-targets

clean: clean-java clean-latex

# Java

java-targets: $(FIRST_TARGET) $(SECOND_TARGET)

$(FIRST_TARGET): $(FIRST_FILES)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(JC) $(JFLAGS) -d $(BIN_DIR) -cp $(BIN_DIR) -sourcepath $(SRC_DIR) $(SRC_DIR)/$(FIRST_PACKAGE)/$(@F:.class=.java)

$(SECOND_TARGET): $(SECOND_FILES)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(JC) $(JFLAGS) -d $(BIN_DIR) -cp $(BIN_DIR) -sourcepath $(SRC_DIR) $(SRC_DIR)/$(SECOND_PACKAGE)/$(@F:.class=.java)

clean-java:
	$(RM) $(BIN_DIR)/$(FIRST_PACKAGE)/*.class
	$(RM) $(BIN_DIR)/$(SECOND_PACKAGE)/*.class

# LaTeX

latex-targets: $(LATEX)

$(LATEX): $(LATEX:.pdf=.tex)
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make $(CURR_DIR)/$(@F:.pdf=.tex)

clean-latex:
	latexmk -C


