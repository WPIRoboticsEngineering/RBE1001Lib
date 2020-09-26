#!/bin/bash

#template
read -r -d '' MANIFEST_STRUCT << EOM
typedef struct
{
  const char * name;
  const char * data;
  const char * mime;
  int length;
} PACKED_FILE;


// Packed Files
EOM

read -r -d '' MANIFEST_PRE << EOM

static const PACKED_FILE static_files_manifest[] =
{
EOM

read -r -d '' MANIFEST_POST << EOM

};
EOM

#Clear out static file.
echo "$MANIFEST_STRUCT" > static.h

#Collect up our static documents
find . -type f -not -name "*.h" -not -name "*.sh" -not -name "*.magic" -print0 | while read -r -d '' file_name
do
  echo "Packing $file_name"
  #Convert to C array and append to packed file.
  #Make sure files terminate with a 0 so they're valud strings.
  #Make sure the array type is static const.
  #Append to static files
  xxd -i $file_name | sed --expression 's/unsigned/static const/g' | sed --expression 's/};/,0x00};/g' | cat >> static.h
done

# Build manifest
echo >> static.h
echo >> static.h
echo "// Manifest Strings" >> static.h
echo >> static.h
# count Files
FILE_COUNT=`cat static.h | grep "static const char __" | awk '{print $4}' | sed "s/\[\]//" | wc -l`

printf "static const int static_files_manifest_count = %s;\n" "$FILE_COUNT" >> static.h


#const strings for filenames.
for f in `cat static.h | grep "static const char __" | awk '{print $4}' | sed "s/\[\]//"`
do
  FNAME=`echo $f | sed "s/__//" | sed "s/_/./g" | cat`
  CNAME=$f
  printf "static const char _filename%s[] = \"/%s\"; \n" "$CNAME" "$FNAME" >>static.h
done

#const strings for file MIME.

echo >> static.h
echo >> static.h
echo "// MIME Strings" >> static.h
echo >> static.h


# Thank You https://github.com/cweiske/MIME_Type_PlainDetect for the awesome magic file.
for f in `cat static.h | grep "static const char __" | awk '{print $4}' | sed "s/\[\]//"`
do
  FNAME=`echo $f | sed "s/__//" | sed "s/_/./g" | cat`
  CNAME=$f
  case "$(echo $FNAME | awk -F. '{print $NF}' )" in
    html) FMIME="text/html" ;;
    js)   FMIME="text/javascript" ;;
    css)   FMIME="text/css" ;;
    *) FMIME="$(file -m programming.magic --mime-type $FNAME | cut -d ' ' -f2)"
  esac
  echo $FMIME
  printf "static const char _MIME%s[] = \"%s\"; \n" "$CNAME" "$FMIME" >>static.h
done


echo >> static.h
echo >> static.h
echo "// Manifest" >> static.h
echo >> static.h
echo "$MANIFEST_PRE" >> static.h

for f in `cat static.h | grep "static const char __" | awk '{print $4}' | sed "s/\[\]//"`
do

  CNAME=$f
  printf "\t{_filename%s,\t %s,\t_MIME%s,\t%s_len}, \n" "$CNAME" "$CNAME" "$CNAME" "$CNAME"  >>static.h
done

echo "$MANIFEST_POST" >> static.h
