
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

# Different JVMs

openjdk7: JC = /usr/lib/jvm/java-7-openjdk-armhf/bin/javac
openjdk8: JC = /usr/lib/jvm/java-8-openjdk-armhf/bin/javac
oracle7: JC = /usr/lib/jvm/jdk-7-oracle-armhf/bin/javac
oracle8: JC = /usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/bin/javac

openjdk7: java-targets
openjdk8: java-targets
oracle7: java-targets
oracle8: java-targets

# These work on my RPi

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


