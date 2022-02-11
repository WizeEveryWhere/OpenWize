#! /bin/bash

#####################################
aLoremIpsum="";
HeaderFile="loremIpsum.h";

#####################################

# echo $aLoremIpsum;

function genPart
{
    local pNum=$1;
    local pSz=$2;
    myVar=`echo "${aLoremIpsum:0:$pSz}"`;
    #echo $myVar;
    myName="aLoremIpsum_p${pNum}";
    mySize=${#myVar};
    myHash=($(echo -n $myVar | sha256sum));
    myHex=$(echo -n $myHash | xxd -r -p);
    
    echo "">> ${HeaderFile};
    echo "//*************************">> ${HeaderFile};
    echo "uint32_t ${myName}_Sz = ${mySize};">> ${HeaderFile};
    echo "">> ${HeaderFile};
    echo "// ${myName}_Hash : $myHash" >> ${HeaderFile};
    echo "uint8_t ${myName}_Hash[32] = {">> ${HeaderFile};
    echo -n $myHex | xxd -i >> ${HeaderFile};
    echo "};" >> ${HeaderFile};
    echo "">> ${HeaderFile};
};

function genFirst
{
    myName="aLoremIpsum";
    mySize=${#aLoremIpsum};
    echo "" >> ${HeaderFile};
    echo "//*************************">> ${HeaderFile};
    echo "const uint32_t ${myName}_Sz = ${mySize};">> ${HeaderFile};
    echo "uint8_t ${myName}[] __attribute__ ((aligned (8))) = {">> ${HeaderFile};
    echo "\"${aLoremIpsum}\"">> ${HeaderFile};
    echo "};">> ${HeaderFile};
    echo "">> ${HeaderFile};
}

# How to : 
echo -n "" > ${HeaderFile};
aLoremIpsum=$(cat ./Latin-Lipsum.txt);

genFirst;
genPart 1 420;
genPart 2 1050;
genPart 3 4200;
genPart 4 28770;
