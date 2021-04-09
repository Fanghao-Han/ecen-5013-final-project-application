#!/bin/sh


usage() {
    FILESDIR=$1
    SEARCHSTR=$2
    echo "[ $0 $1 $2 ]"
    echo "Usage: $0 [SEARCHDIR] [SEARCHSTRING]"
    echo "Search for files in the target directory that contain the given string"
    echo
}


# Check that requied args are given
if [ $# -ne 2 ]; then usage; exit 1; fi;


filesdir=$1
searchstr=$2

if [ -d $filesdir ]&&[ $searchstr ]
then
    fileNum=$( ls $filesdir | wc -l )	
    str1="The number of files are $fileNum "
    #matchNum=$( grep -R $searchstr $filesdir | wc -l )
    matchNum=$( grep -r $searchstr $filesdir | wc -l )
    str2="and the number of matching lines are $matchNum"
    echo $str1$str2
else	
	str1="invalid arguments"
	
	if [ ! -d $filesdir ]
	then
	str2=" - invalid directory"
	fi
	
	echo $str1$str2
	exit 1	
fi




