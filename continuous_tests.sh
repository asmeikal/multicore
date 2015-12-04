#!/bin/bash

ljvm=["oracle7","oracle8","openjdk7","openjdk8"]
limp=["prima","seconda"]
BIN="./bin"

oracle7="a"
oracle8="b"
openjdk7="c"
openjdk8="d"

prima="homework_prima_implementazione/MainClass.class"
seconda="homework_seconda_implementazione/MainClass.class"

wait_min() {
	secs=$(($1 * 60))
	echo -e "\nWaiting $1 minute"
	while [ $secs -gt 0 ]; do
		echo -ne "$secs\033[0K\r"
		sleep 1
		: $((secs--))
	done
	echo "done"
}

while read impl jvm args; do
	if [[ ! $impl ]] ; then continue ; fi
	wait_min 1
	echo ""
	echo "$impl $jvm $args"
	time=`date +"%F-%H-%M"`
	fname="${impl}_${jvm}_${time}.log"
	echo "$impl $jvm $args" > $fname
	echo "make clean-java" >> $fname
	make clean-java >> $fname 2>&1
	if [ $? -ne 0 ] ; then continue ; fi
	echo "make $jvm" >> $fname
	make $jvm >> $fname 2>&1
	if [ $? -ne 0 ] ; then continue ; fi
	echo "${!jvm} -cp $BIN ${!impl} $args" >> $fname
	{ /usr/bin/time -v ${!jvm} -cp $BIN ${!impl} $args ; } >> $fname 2>&1
done <$1

echo "All work is done"

