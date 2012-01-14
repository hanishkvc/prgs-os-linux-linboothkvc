#!/bin/bash
# Logic to help release the project internally (myself) or externally
# HKVC, GPL, Jan2012

#rdir=linboothkvc-`date +%F-%T`
rdir=linboothkvc-v`date +%F-%H`
echo $rdir
read -p "Press any key to continue..."
mkdir $rdir

echo "Files being copied..."
cp -v *.* $rdir/
cp -v Makefile $rdir/
cp -v README* $rdir/

if [[ $1 != "internal" ]]; then
echo "INFO: EXTERNAL release - Shorten Developer full name..."
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
rm -rf release.sh
grep -i "hanis" * | less
cd ..
else
echo "INFO: INTERNAL release - Developer full name not shortened"
fi

zip -r $rdir.zip $rdir
read -p "Press any key to remove $rdir"
rm -rf $rdir

