
.PHONY: $(TARGETS)
JC=javac
JFLAGS=-g

CURR_DIR=.
SRC_DIR=$(CURR_DIR)/src
BIN_DIR=$(CURR_DIR)/bin
MAIN_PACKAGE=homework_prima_implementazione
PACKAGES=$(MAIN_PACKAGE)

MAIN_FILES=$(SRC_DIR)/$(MAIN_PACKAGE)/*.java
TARGETS=$(BIN_DIR)/$(MAIN_PACKAGE)/MainClass.class

$(TARGETS): $(MAIN_FILES)
	test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(JC) $(JFLAGS) -d $(BIN_DIR) -cp $(BIN_DIR) -sourcepath $(SRC_DIR) $(SRC_DIR)/$(MAIN_PACKAGE)/$(@F:.class=.java)

clean:
	$(RM) $(BIN_DIR)/$(PACKAGES)/*.class

