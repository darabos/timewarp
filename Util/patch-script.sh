#!/bin/sh

if [ $2 ]; then

NEW_FILELIST=`find $2 -type f "(" -name "*.*" ")" | grep -v CVS[/]`;
OUTDIR=TIMEWARP_TEMPDIR;
OUTFILE=`date +%D | sed s/[/]/_/g`;

mkdir $OUTDIR;

   for file in $NEW_FILELIST; do
	basename_file=`ls $file | sed s/$2[/]//g`;
	
	dif=`diff -q -P -N $1/$basename_file $2/$basename_file 2>&1`;
	
	if [ "$dif" ]; then
		cd $2;
		cp --parents $basename_file ../$OUTDIR;
		cd ..;
	fi
   done

cd $OUTDIR;
tar -cf ../$1_to_$OUTFILE.tar *;
cd ..;
gzip -f -9 $1_to_$OUTFILE.tar;
rm -f -r $OUTDIR;

else
	echo "Usage: patch-script.sh OLD_DIR NEW_DIR";
fi 

