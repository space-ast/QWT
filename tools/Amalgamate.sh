#!/bin/bash

DEST=../src
OPTS='-i "../src" -i "../src/core" -i "../src/3rdparty/gl2ps" -w "*.cpp;*.h;*.hpp;*.c" -s'
./Amalgamate.exe $OPTS ./amalgamate/QWTAmalgamTemplate.h $DEST/../src-amalgamate/QwtPlot.h
./Amalgamate.exe $OPTS ./amalgamate/QWTAmalgamTemplate.cpp $DEST/../src-amalgamate/QwtPlot.cpp

# Remove the line containing #include "gl2ps.h" from QwtPlot.cpp
if [ -f "$DEST/../src-amalgamate/QwtPlot.cpp" ]; then
    sed -i '/#include "gl2ps.h"/d' "$DEST/../src-amalgamate/QwtPlot.cpp"
    echo "Removed #include \"gl2ps.h\" from QwtPlot.cpp"
    
    # Convert line endings from LF to CRLF
    convert_to_crlf() {
        local file="$1"
        if [ -f "$file" ]; then
            awk '{sub(/$/, "\r"); print}' "$file" > "${file}.tmp"
            mv "${file}.tmp" "$file"
            echo "Converted line endings to CRLF for $file"
        fi
    }
    
    convert_to_crlf "$DEST/../src-amalgamate/QwtPlot.cpp"
    convert_to_crlf "$DEST/../src-amalgamate/QwtPlot.h"
else
    echo "Warning: QwtPlot.cpp file does not exist"
fi

# Use read command to achieve functionality similar to pause in batch files
echo Press any key to continue...
read -n 1
echo Continuing...