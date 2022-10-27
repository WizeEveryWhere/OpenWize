#!/bin/bash

tool_dir="../../../../../tools/scripts/decomp_support"
app_dir="../golden"
gold_dir="../golden"

${tool_dir}/lzma_script.sh ${app_dir}/app.bin;

gcc -c _img.c;
objcopy --dump-section .rodata=_img.ro _img.o;
cat _img.ro ${app_dir}/app.bin.lzma > ${gold_dir}/outfile.bin;
rm _img.ro _img.o;


