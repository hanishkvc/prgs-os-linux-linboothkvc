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
	sed -e "s/hanishkvc/hkvc/" temp_9999 > $i
	diff -ub temp_9999 $i | less
done
rm temp_9999
cd ..
zip -r $rdir.zip $rdir
read -p "Press any key to remove $rdir"
rm -rf $rdir

