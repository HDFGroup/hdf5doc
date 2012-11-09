#!/bin/sh

#  h4_compress_test.sh -- script to run hrepack with all possible settings
#     for GZIP and SZIP compression.
#
#   input:  one or more HDF4 files
#   output: size of compressed file, compression and decompression time
#
#   Note: this calls hrepack.  See the hrepack documentation.

### set options
#
#
#  Set workdir to a scratch area
#
WORKDIR=/var/tmp/W

#
#  Set these paths
#
HREPACK=/afs/ncsa.uiuc.edu/projects/hdf/java/java6/mcgrath/verbena/Build4/bin/hrepack

#
#  Optionally, manually set this to the list of files, or pass in as arguments
#
#FILELIST=szip_tfile.hdf
FILELIST=$*

#
#  set verbose to "yes" to get a lot more output!
#
#verbose="yes"
verbose="no"


####
#  Don't edit below here
####

if [ ! -f $HREPACK ];
then
	echo "hrepack not found at: "$HREPACK
	exit 1
fi

if [ "$FILELIST" = "" ];
then
	echo "Usage: h4_compression_test.sh file.hdf [file2.hdf ...]"
	echo "   or edit the script to set FILELIST"
	exit 1
fi

if [ ! -d $WORKDIR ];
then
   mkdir $WORKDIR
   if [ "$?" != "0" ];
   then
	echo "could not create work dir: "$WORKDIR
	exit 1
   fi
fi

SZIP_SETTINGS="2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32"
SZIP_MODE="EC NN"
GZIP_LEVEL="1 2 3 4 5 6 7 8 9"

if [ "$verbose" != "no" ];
then
	VFLAG="-v"
else
	VFLAG=""
fi

for f in $FILELIST
do

  ferrs=0;

  if [ ! -f "$f" ];
  then
	echo "File not found? "$f
	exit 1;
  fi
  echo "Copying $f to scratch area"
  cp $f $WORKDIR
  res="$?"
  if [ "$res" != "0" ];
  then
	echo "Copy failed "$f "->"$WORKDIR
	exit 1;
  fi
  FILE=$WORKDIR/`basename $f`
  echo "Testing "$FILE

  orig_size=`wc -c $FILE | awk '{print $1}'`

  #
  #  SZIP tests: try all combinations of settings for szip
  #
  ofile1=$FILE"_tab1"
  echo "Results for SZIP $FILE (size $orig_size)" > $ofile1.txt
  echo "parms	comp	amount	ctime	uctime" >> $ofile1.txt
  for ss in $SZIP_SETTINGS
  do 
    for mm in $SZIP_MODE
    do 
        compressed_size="-1"
        amt_comp="0"
        comp_time="-1"
        uncomp_time="-1"
        s="$ss,$mm"
        cond=$ss"_"$mm
        cerrs=0;
        echo "   SZIP "$s"  "
        # compress with szip
        (/usr/bin/time -p $HREPACK $VFLAG -i $FILE -o $FILE.out.$s.1 -t "*:SZIP $s") 2> $WORKDIR/ct

        res="$?"
        if [ "$res" != "0" ]; 
	then
		echo "repack command failed:"
        	echo "$HREPACK $VFLAG -i $FILE -o $FILE.out.$s.1 -t \"*:SZIP $s\""
		ferrs=`expr $ferrs + 1`
		cerrs=`expr $cerrs + 1`
echo $cond"	""***""	""***""	""***""	""***" >> $ofile1.txt
		continue;

	fi
        comp_time=`cat $WORKDIR/ct | awk '/eal/{print $2}'`
        rm $WORKDIR/ct
        compressed_size=`wc -c $FILE.out.$s.1 | awk '{print $1}'`

        amt_comp=`expr $orig_size - $compressed_size`
        comp_ratio=`expr $orig_size - $compressed_size`

        # uncompress the data
        (/usr/bin/time -p $HREPACK $VFLAG -i $FILE.out.$s.1 -o $FILE.out.$s.2 -t '*:NONE') 2> $WORKDIR/ct
        res="$?"
        if [ "$res" != "0" ]; 
	then
		echo "repack uncompress command failed:"
                echo "$HREPACK $VFLAG -i $FILE.out.$s.1 -o $FILE.out.$s.2 -t '*:NONE'"
		ferrs=`expr $ferrs + 1`
		cerrs=`expr $cerrs + 1`
                echo $cond"	"$compressed_size"	"$amt_comp"	"$comp_time"	""***" >> $ofile1.txt
		continue;
        fi
        uncomp_time=`cat $WORKDIR/ct | awk '/eal/{print $2}'`
        rm $WORKDIR/ct

        echo $cond"	"$compressed_size"	"$amt_comp"	"$comp_time"	"$uncomp_time >> $ofile1.txt
        
        # rm all files
        rm $FILE.out.$s.1 $FILE.out.$s.2 
        if [ "$cerrs" != "0" ] ;
        then
	        echo "    $FILE $s: Failed"
        fi
    done
  done

  ofile2=$FILE"_tab2"
  echo "Results for GZIP" > $ofile2.txt
  echo "parms	comp	amount	ctime	uctime" >> $ofile2.txt
  for ll in $GZIP_LEVEL
    do 
        echo "   GZIP "$ll"  "
        # compress with szip
        (/usr/bin/time -p $HREPACK $VFLAG -i $FILE -o $FILE.out.$ll.1 -t "*:GZIP $ll") 2> $WORKDIR/ct
        res="$?"
        if [ "$res" != "0" ]; 
	then
	    echo "repack command failed:"
            echo "$HREPACK $VFLAG -i $FILE -o $FILE.out.$ll.1 -t \"*:GZIP $ll\""
	    ferrs=`expr $ferrs + 1`
	    cerrs=`expr $cerrs + 1`
            echo $ll"	""***""	""***""	""***""	""***" >> $ofile2.txt
	    continue;
	fi
        comp_time=`cat $WORKDIR/ct | awk '/eal/{print $2}'`
        rm $WORKDIR/ct
        compressed_size=`wc -c $FILE.out.$ll.1 | awk '{print $1}'`

        amt_comp=`expr $orig_size - $compressed_size`
        comp_ratio=`expr $orig_size - $compressed_size`

        # uncompress the data
        (/usr/bin/time -p $HREPACK $VFLAG -i $FILE.out.$ll.1 -o $FILE.out.$ll.2 -t '*:NONE') 2> $WORKDIR/ct
        res="$?"
        if [ "$res" != "0" ]; 
	then
           echo "repack uncompress command failed:"
           echo "$HREPACK $VFLAG -i $FILE.out.$ll.1 -o $FILE.out.$ll.2 -t '*:NONE'"
	   ferrs=`expr $ferrs + 1`
	   cerrs=`expr $cerrs + 1`
           echo $cond"	"$compressed_size"	"$amt_comp"	"$comp_time"	""***" >> $ofile2.txt
	   continue;
        fi
        uncomp_time=`cat $WORKDIR/ct | awk '/eal/{print $2}'`
        rm $WORKDIR/ct

        echo $ll"	"$compressed_size"	"$amt_comp"	"$comp_time"	"$uncomp_time >> $ofile2.txt
        
        # rm all files
        rm $FILE.out.$ll.1 $FILE.out.$ll.2 
        if [ "$cerrs" != "0" ] ;
        then
	        echo "    $FILE $s: Failed"
        fi
    done

  rm $FILE
  if [ "$ferrs" != "0" ] ;
  then
	echo "$FILE:  *Errors detected*"
  else
	echo $FILE":   OK"
	echo
	cat $ofile1.txt
	echo
	cat $ofile2.txt
  fi
done

exit $ferrs
