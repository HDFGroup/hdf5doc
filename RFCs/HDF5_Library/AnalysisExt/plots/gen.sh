#!/bin/bash
for i in `ls *.gpi`
do
    echo $i
    gnuplot $i
done

