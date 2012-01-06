#!/bin/bash

#rdir=linboothkvc-`date +%F-%T`
rdir=linboothkvc-v`date +%F-%H`
echo $rdir
read -p "Press any key to continue..."
mkdir $rdir
cp *.* $rdir/
cp Makefile $rdir/
#rm $rdir/release.sh
cd $rdir
for i in *; do
	echo $i
	mv $i temp_9999
	sed -e "s/hanis/hkvc/" temp_9999 > $i
	diff -ub temp_9999 $i
	if [[ "$?" != "0" ]]; then
		diff -ub temp_9999 $i | less
	fi
done
rm temp_9999
grep -i "hanis" * | less
cd ..
zip -r $rdir.zip $rdir
read -p "Press any key to remove $rdir"
rm -rf $rdir

