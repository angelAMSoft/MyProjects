#!/bin/sh

mkdir "/tmp/netlist"
sudo lshw -C network > /tmp/netlist/tmp1
cat /tmp/netlist/tmp1 | grep -E 'продукт*|логическое*|product*|logical*' > /tmp/netlist/tmp2
rm /tmp/netlist/tmp1
awk -F: '{print $2}' /tmp/netlist/tmp2 > /tmp/netlist/tmp3
rm /tmp/netlist/tmp2

while read rec1
do
  read rec2
  echo "$rec2 $rec1"
done < /tmp/netlist/tmp3

rm /tmp/netlist/tmp3
rmdir "/tmp/netlist"
