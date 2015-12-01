#/bin/bash

BIN_DIR="bin"
TARGET="homework_prima_implementazione.MainClass"
if [[ -x "/usr/bin/time" ]] ; then
	TIME="/usr/bin/time"
	TIME_FLAGS="-v"
else
	TIME="time"
fi

make java-targets
echo "java -classpath $BIN_DIR $TARGET $1 $2"
($TIME $TIME_FLAGS 2>&1 java -classpath $BIN_DIR $TARGET $1 $2) | tee output.txt

