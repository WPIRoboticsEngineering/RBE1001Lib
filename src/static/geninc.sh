#!/bin/bash

# Run this script to generate include files

xxd -i index.html indexhtml.h
sed -i 's/};/,0x00};/g' indexhtml.h
sed -i 's/unsigned/static/g' indexhtml.h

xxd -i nipplejs.min.js nipplejsminjs.h
sed -i 's/};/,0x00};/g' nipplejsminjs.h
sed -i 's/unsigned/static/g' nipplejsminjs.h