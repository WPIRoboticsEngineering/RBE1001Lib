#!/bin/bash

# Run this script to generate include files

xxd -i index.html indexhtml.h
sed -i 's/};/,0x00};/g' indexhtml.h
sed -i 's/unsigned/static const/g' indexhtml.h

xxd -i nipplejs.min.js nipplejsminjs.h
sed -i 's/};/,0x00};/g' nipplejsminjs.h
sed -i 's/unsigned/static const/g' nipplejsminjs.h

xxd -i chart.js chartjs.h
sed -i 's/};/,0x00};/g' chartjs.h
sed -i 's/unsigned/static const/g' chartjs.h

xxd -i menu.js menujs.h
sed -i 's/};/,0x00};/g' menujs.h
sed -i 's/unsigned/static const/g' menujs.h

xxd -i robot.js robotjs.h
sed -i 's/};/,0x00};/g' robotjs.h
sed -i 's/unsigned/static const/g' robotjs.h

xxd -i style.css sytlecss.h
sed -i 's/};/,0x00};/g' sytlecss.h
sed -i 's/unsigned/static const/g' sytlecss.h
