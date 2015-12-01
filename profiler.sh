#/bin/bash

BIN_DIR="bin"
TARGET="homework_prima_implementazione.MainClass"

if [ ! -d $BIN_DIR ] ; then
	mkdir -p $BIN_DIR
fi
make
echo "java -classpath $BIN_DIR $TARGET $1 $2"
/usr/bin/time java -classpath $BIN_DIR $TARGET $1 $2 | tee output.txt

