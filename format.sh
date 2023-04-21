#!/bin/bash

function format()
{
    find "$2" | grep "$1$" | grep "^$2" | while read name
    do
        clang-format $name > ./tmp.txt
        mv ./tmp.txt $name
        echo "Processing $name"
    done
}

format ".cpp" "./src"
format ".hpp" "./include/Sharpen"
format ".cpp" "./test"
echo "Done"