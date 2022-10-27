#!/bin/bash

filesize=$(stat -c %s $1);

hex_sz=$(printf "%016x\n" "${filesize}");
le_hex_sz=$( echo ${hex_sz} | rev | dd conv=swab 2>/dev/null);

echo "-> $1 filesize = ${filesize} (0x${hex_sz})";
#echo ${le_hex_sz};

echo "-> lzma compress $1 into $1.lzma";
#lzma -z -0 $1 -c --lzma1="dict=64KiB,lc=3,lp=0,pb=2" > $1.lzma
# state_size = 16KB

lzma -z -0 $1 -c --lzma1="dict=8KiB,lc=3,lp=0,pb=2" > $1.lzma
# state_size = 16KB

#lzma -z -0 $1 -c --lzma1="dict=8KiB,lc=3,lp=0,pb=1" > $1.lzma
# state_size = 16KB

echo "-> add size info in lzma header";
echo ${le_hex_sz} | xxd -r -p | dd of=$1.lzma bs=1 seek=5 count=8 conv=notrunc 2>/dev/null ;
hexdump -n8 -s5 -e '"%08_ax" 16/1 " %02x" "\n"' $1.lzma;

echo "";
printf "** lzma info header **";
lzmainfo $1.lzma;
