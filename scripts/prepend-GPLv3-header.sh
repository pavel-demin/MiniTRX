#! /bin/sh

header=scripts/GPLv3-header.txt
length=`wc -l $header | cut -d' ' -f1`

for file in `find client server -maxdepth 2 -type f -name *.cpp -o -name *.h`
do
  head -$length $file | diff -lb $header - > /dev/null && continue
  echo $file
  cat $header $file > $file.new
  mv $file.new $file
done
