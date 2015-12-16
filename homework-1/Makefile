
.PHONY: all
main: all

JC=javac
JFLAGS=-g

CURR_DIR=.
SRC_DIR=$(CURR_DIR)/src
BIN_DIR=$(CURR_DIR)/bin

SECOND_PACKAGE=homework_seconda_implementazione
SECOND_FILES=$(SRC_DIR)/$(SECOND_PACKAGE)/*.java
MAIN_TARGET=$(BIN_DIR)/$(SECOND_PACKAGE)/Main.class

all: clean-java java-targets

clean: clean-java

# Java

java-targets: $(MAIN_TARGET)

$(MAIN_TARGET): $(SECOND_FILES)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(JC) $(JFLAGS) -d $(BIN_DIR) -cp $(BIN_DIR) -sourcepath $(SRC_DIR) $(SRC_DIR)/$(SECOND_PACKAGE)/$(@F:.class=.java)

clean-java:
	$(RM) $(BIN_DIR)/$(SECOND_PACKAGE)/*.class


