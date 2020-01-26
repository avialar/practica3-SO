#!/usr/bin/zsh -f
USAGE="USAGE\n\t$0 <num>"
ver="2\n1"
todo="2\n1\nn\n5\n5"

if [ $# != 1 ]
then
	echo -e "$USAGE"
	exit 1
fi

for i in $(seq 1 1 $1)
do
	echo "$ver" | ./cliente&
done
