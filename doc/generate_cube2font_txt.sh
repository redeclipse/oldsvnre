#!/bin/sh
man --no-justification --no-hyphenation man/cube2font.1 | col -b > cube2font.txt

cat >> $(dirname $0)/cube2font.txt << EOF

This text file was automatically generated from cube2font.1
Please do not edit it manually.
EOF

