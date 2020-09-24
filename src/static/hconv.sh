#!/bin/bash

echo "" > static.h
find . -type f -not -name "*.h" -not -name "*.sh"  -print0 | while read -r -d '' file_name
do
  echo "Packing $file_name"
  xxd -i $file_name | sed --expression 's/};/,0x00};/g'\
   | sed --expression 's/unsigned/static const/g' | cat >> static.h
done
# test.h
