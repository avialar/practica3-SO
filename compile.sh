USAGE="USAGE\n\t$0 \"mensaje de commit\""
if [ $# != 1 ];
then
	echo -e $USAGE
	exit 1
fi

git add -A
git commit -m "$1"
git checkout master
git pull
git merge local
git push
git checkout -B local
