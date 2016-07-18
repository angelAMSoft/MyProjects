#!/bin/sh

route > /tmp/routes
echo "Search default route..."
iface=`grep default /tmp/routes | awk '{print $5,$8}' | sort -n | awk '{print $2}' | sed -n '1p'`
echo "Analyze packets sended to 8.8.8.8"
sudo tcpdump -i $iface ip -c 2 dst host 8.8.8.8 -n -q -t > /tmp/t 2>/dev/null &
sleep 1 
telnet 8.8.8.8 53 > /dev/null 2>/dev/null
echo "-----------------------------------------------------------"
echo "Result IP:"
grep IP /tmp/t | uniq | awk '{print $2}' | sed -n 's/[0-9]\{,3\}.[0-9]\{,3\}.[0-9]\{,3\}.[0-9]\{,3\}/ip & port/p' | awk '{print $2}' | uniq
echo "-----------------------------------------------------------"
sleep 1
echo "Cleaning temp files..."
rm /tmp/t
rm /tmp/routes
