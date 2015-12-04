#!/bin/bash

BIN="./bin"

source jvms.sh

prima="homework_prima_implementazione/MainClass"
seconda="homework_seconda_implementazione/MainClass"

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
	echo ""
	time=`date +"%F-%H-%M"`
	fname="${impl}_${jvm}_${time}.log"
	if [ -e "./$fname" ] ; then
		wait_min 1
		time=`date +"%F-%H-%M"`
		fname="${impl}_${jvm}_${time}.log"
	fi
	echo "$impl $jvm $args"
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

echo -e "\nAll work is done\n"

