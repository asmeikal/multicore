#!/bin/bash

if [[ $# != 1 ]] ; then
	echo "Fornire un nome di file."
	exit 1
fi

if [[ ! -f $1 ]] ; then
	echo "'$1' non sembra essere un file regolare."
	exit 1
fi

echo "" > global.txt
echo "" > local.txt
echo "" > unlimited.txt
echo "" > cpu.txt

for i in `seq 1 2 69`; do
	echo "Testing filter size $i"
	./homework_global $1 $i > output.txt 2>&1
	if [[ $? != 0 ]] ; then
		echo "Error at filter size $i"
		exit 1
	fi
	mv output.png check.png
	echo "Testing global"
	for j in `seq 16`; do
		./homework_global $1 $i > output.txt 2>&1
		if [[ $? != 0 ]] ; then
			echo "Global homework error"
			echo "$i ERROR" >> global.txt
		else
			diff check.png output.png
			if [[ $? != 0 ]] ; then
				echo "Global image differs!"
				echo "$i IMAGES DIFFER" >> global.txt
			else
				tail -n 1 output.txt >> global.txt
			fi
		fi
	done
	echo "Testing local"
	for j in `seq 16`; do
		./homework_local $1 $i > output.txt 2>&1
		if [[ $? != 0 ]] ; then
			echo "Local homework error"
			echo "$i ERROR" >> local.txt
		else
			diff check.png output_local.png
			if [[ $? != 0 ]] ; then
				echo "Local image differs!"
				echo "$i IMAGES DIFFER" >> local.txt
			else
				tail -n 1 output.txt >> local.txt
			fi
		fi
	done
	echo "Testing local unlimited"
	for j in `seq 16`; do
		./homework_unlimited $1 $i > output.txt 2>&1
		if [[ $? != 0 ]] ; then
			echo "Local unlimited homework error"
			echo "$i ERROR" >> unlimited.txt
		else
			diff check.png output_unlimited.png
			if [[ $? != 0 ]] ; then
				echo "Local unlimited image differs!"
				echo "$i IMAGES DIFFER" >> unlimited.txt
			else
				tail -n 1 output.txt >> unlimited.txt
			fi
		fi
	done
	echo "Testing CPU"
	for j in `seq 8`; do
		./homework_cpu $1 $i > output.txt 2>&1
		if [[ $? != 0 ]] ; then
			echo "CPU homework error"
			echo "$i ERROR" >> cpu.txt
		else
			diff check.png output.png
			if [[ $? != 0 ]] ; then
				echo "CPU image differs!"
				echo "$i IMAGES DIFFER" >> cpu.txt
			else
				tail -n 1 output.txt >> cpu.txt
			fi
		fi
	done
done

