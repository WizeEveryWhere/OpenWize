
echo "Create dir $CWD";
mkdir -p $CWD;
rm -rf $CWD/CMakeFiles $CWD/*.cmake $CWD/*.txt
echo "Create app file : $1";
echo "$1" > $CWD/app
