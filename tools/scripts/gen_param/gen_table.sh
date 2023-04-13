#!/bin/bash 

################################################################################
bCMD="xmlstarlet sel -t -v"


gParam=""
gSortedID=""
gStr=""

re='^[A-Fa-f0-9x]+$'

# Get the attribute value
# in : the xml file name
# in : the field name
# in : the attribute name
# in : the field id
# out : the attribute value (result in gParam)
function getAttr {
    local mFile=$1
    local field=$2; # field name
    local attr=$3;  # attribute name
    local id=$4;    # field id
    local var="";
    var=`${bCMD} "//${field}[@id=\"${id}\"]/@${attr}" -n ${mFile}`;
    # remove CRLF
    gParam=$(echo ${var}|tr -d '\n')
}

# Get the attribute ID value in list in decending order
# in : the xml file name
# in : the field name
# out : the ID list (result in gSortedID)
function getSortedID {
    local mFile=$1
    local field=$2; # field name
    xmlstarlet sel -t -m "//${field}" -s A:T:L '@id'  -v '@id' -n ${mFile} > .temp.txt   
    gSortedID=(`cat .temp.txt`)  
    rm -f .temp.txt    
}

# Print the footer into the given file
# in : the file name to print into
# in : the string to print
function printFooter {
    local toFile=$1;
    local str=$2;
    printf "%s\n" "${str}">> ${toFile};
}

# Print the head of source and header file
# in : the file generated from
# in : the file where to write in
function printHeaders {
    local baseFile=$1;
    local toFile=$2;
    # print header head
    local now=$(date '+%d/%m/%Y %H:%M');
    
cat << EOF > ${toFile}
/*!
  * @file $(basename ${toFile})
  * @brief This file was generated from ${baseFile}.
  * 
  * @details
  *
  * @copyright 2023, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *
  * @par Generation Date
  * 
  * @par x.x.x : ${now} [${USER}]
  *
  */

EOF

}

#===============================================================================

# Print one line of access table (in the source file)
# in : the source file name
# in : the parameter id
# in : the local access
# in : the remote access
# in : the effectivness
# in : the ref as referenced
# in : the size of parameter
# in : the offset in value table
# in : the restr as restriction id
function printAccess {
    local sFile=$1;
    local mID=$2;
    local loc=$3;
    local rem=$4;
    local eff=$5;
    local ref=$6;
    local size=$7;
    local offset=$8;
    local restr=$9;
    printf "    INIT_ACCESS_TABLE(0x%s, %s, %s, %s, %s, %d, %d, 0x%s),\n" ${mID} ${loc} ${rem} ${eff} ${ref} ${size} ${offset} ${restr} >> ${sFile};
}

# Print one line of ids table (in the header file)
# in : the header file name
# in : the parameter id
# in : the name of parameter
# in : the description of parameter
function printIDs {
    local hFile=$1;
    local mID=$2;
    local name=$3;
    local desc=$4;
    #printf "    %s = 0x%s, //!< %s\n" "${name}" "${mID}" "${desc}" >> ${hFile};
    #printf "    %s = 0x%s, /*!< %s */\n" "${name}" "${mID}" ${desc[@]} >> ${hFile};
    printf "    %s = 0x%s, " "${name}" "${mID}" >> ${hFile};
    printf "/*!< %s */\n" "${desc}" >> ${hFile};
}

# Print one line of parameter value table (in the value file)
# in : the value file name
# in : the parameter id
# in : the value of parameter
# in : the size of parameter
# in : the description of parameter
function printVal {        
    local vFile=$1;
    local value=$2;
    local size=$3;
    local desc=$4;
    local i=0;
    #echo "value = ${value}; size = ${size}"
    #printf "value = ${value}; size = ${size}\n"

    if ! [[ ${value} =~ ${re} ]] # It's an hex format value 
    then
        #echo "value = ${value}; size = ${size}"
        for i in $(seq 0 $((${size} -1)) )
        do
            s=${value:$i:1};
            if [[ ${s} != "" ]]
            then
                printf "'%c', " ${s} >> ${vFile};
                #echo "value = ${s}" | hexdump -C
            else
                printf "'%s', " "\0" >> ${vFile};
                echo ""
                #echo "... is a string"
            fi
        done
        #printf " //!< %s\n   " "${desc}" >> ${vFile};
        printf " /*!< %s*/\n   " "${desc}" >> ${vFile};
        #printf "\n" >> ${vFile};
        
    else # It's an decimal format value 
        #echo "value = ${value}; size = ${size}"
        if [[ ${size} -gt 1 ]] 
        then 
            for i in $(seq 1 ${size})
            do           
                j=$((i))*2;  
                vv=$(( 0x${value:$j:2} ));
                #printf "   0x%01x, " ${vv};
                printf "0x%01x," ${vv} >> ${vFile};
            done
        else
            v=$(( ${value} ));
            #echo "v = ${v}"
            for i in $(seq 1 ${size})
            do
                vv=$(( ${v} & 0xff));
                printf "0x%01x," ${vv} >> ${vFile};
                v=$(( ${v} >> 8))
            done
        fi
        printf " /*!< %s*/\n   " "${desc}" >> ${vFile};
        #printf "\n" >> ${vFile};
    fi
}

# Build the Parameters tables :
# - Acces table 
# - Value table
# - Id table (enum of parameters ID)
# in : the xml file 
# in : the acces file 
# in : the value file
# in : the header file
# in : the setup file
function buildParametersTables {
    local mFile=$1;
    local sFile=$2;
    local vFile=$3;
    local hFile=$4;
    local tFile=$5;

    ###########################################################################
    local field="Parameter";
    
    # get sorted id   
    getSortedID "${mFile}" "${field}";
    pID=(${gSortedID[@]})
    
    # get nb id and id max
    idNb=$(( ${#pID[@]} ))
    idMax=${pID[$(( ${idNb} -1 ))]} 
    
    # get nb id and id max as decimal number
    idNb_dec=$(echo "obase=10; ibase=16; ${#pID[@]};" | bc );   
    idMax_dec=$(echo "obase=10; ibase=16; ${pID[$(( ${idNb} -1 ))]};" | bc );
       
    # print access file header
    szMax=$(echo "obase=16; ibase=10; $(( ${idMax_dec} +1));" | bc );
    printf "\n/******************************************************************************/\n" >> ${sFile};
    printf "const param_s a_ParamAccess[0x%s] = {\n" "${szMax}" >> ${sFile};
       
    # print header file header
    printf "\n/******************************************************************************/\n" >> ${hFile};
    printf "#define PARAM_ACCESS_CFG_SZ (0x%s)\n" "${szMax}" >> ${hFile};
    printf "#define PARAM_DEFAULT_SZ (0xUndefSizeForNow)\n\n" >> ${hFile};
    printf "extern const param_s a_ParamAccess[PARAM_ACCESS_CFG_SZ];\n" >> ${hFile};
    printf "extern const uint8_t a_ParamDefault[PARAM_DEFAULT_SZ];\n\n" >> ${hFile};
    
    printf "/*!\n * @brief This enum define the parameter id\n */\n" >> ${hFile};
    printf "typedef enum {\n" >> ${hFile};
    
    # print value file header    
    printf "\n/******************************************************************************/\n" >> ${vFile};
    printf "/*!\n * @brief This array define the parameter default value\n */\n" >> ${vFile};
    printf "const uint8_t a_ParamDefault[] = {\n   " >> ${vFile};
    
    # print setup file header
    if [[ ${generate_setup} == "1" ]]
    then
        printf "\n/******************************************************************************/\n" >> ${tFile};
        printf "\n#ifndef ATTR_PARAM_TABLE\n#define ATTR_PARAM_TABLE()\n#endif\n" >> ${tFile};
        printf "/*!\n * @brief This array define the parameter default value\n */\n" >> ${tFile};
        printf "ATTR_PARAM_TABLE()\n" >> ${tFile};
        printf "const uint8_t aParamSetup[] = {\n   " >> ${tFile};
    fi
    ##
    local i=0;
    local j=0;
    local offset=0
    local mID=0
    for j in $(seq 0 ${idMax_dec})
    do
        pID_dec=$(echo "obase=10; ibase=16; ${pID[$i]};" | bc )
        
        if [[ ${is_silent} == 1 ]]
        then
            printf ".";
            if [[ $(( $j % 50 )) == 0 ]]
            then
                printf "\n";
            fi
            
            if [[ $j == ${idMax_dec} ]]
            then
                printf "\n";
            fi
        fi
        
        #echo "j=$j; i=$i; pID_dec=${pID_dec}; pID[$i]=${pID[$i]}"
        if [[ $j == ${pID_dec} ]]
        then
            getAttr "${mFile}" "${field}" "name"   ${pID[$i]}; name=${gParam};            
            getAttr "${mFile}" "${field}" "size"   ${pID[$i]}; size=$(( ${gParam} ));
            getAttr "${mFile}" "${field}" "loc"    ${pID[$i]}; loc=${gParam};
            getAttr "${mFile}" "${field}" "rem"    ${pID[$i]}; rem=${gParam};
            getAttr "${mFile}" "${field}" "eff"    ${pID[$i]}; eff=${gParam};
            getAttr "${mFile}" "${field}" "desc"   ${pID[$i]}; desc=${gParam};
            getAttr "${mFile}" "${field}" "ref"    ${pID[$i]}; ref=${gParam};
            getAttr "${mFile}" "${field}" "restr"  ${pID[$i]}; restr=${gParam};
            if [[ ${eff} == "" ]]
            then
                eff="IMM";
            fi
            if [[ ${ref} == "" ]]
            then
                ref="REF_Y";
            fi
            if [[ ${restr} == "" ]]
            then
                restr="0";
            fi
            
            mID=${pID[$i]};
            
            # Print in access table
            printAccess "${sFile}" "${mID}" "${loc}" "${rem}" "${eff}" "${ref}" ${size} ${offset} "${restr}"
            
            # Print in parameter id table           
            printIDs "${hFile}" "${mID}" "${name}" "${desc}";

            # Get the parameter value 
            var=`${bCMD} "//${field}[@id=\"${pID[$i]}\"]" -n ${mFile}`
            value=$(echo ${var}|tr -d '\n');
            
            # Print the parameter value 
            printVal "${vFile}" "${value}" ${size} "${desc}";
            
            if [[ ${generate_setup} == "1" ]]
            then
                printVal "${tFile}" "${value}" ${size} "${desc}";
            fi
            
            if [[ ${is_silent} == 0 ]]
            then
                printf "Id [%s] = %s // %s \n" "${mID}" "${value}" "${desc}";
            fi
            
            # Compute the next offset
            offset=$((offset+${size}));
            # Next parameter id
            i=$(($i+1));
        else
            name=""
            desc="Unused"
            mID=$(echo "obase=16; ibase=10; ${j};" | bc );
            
            #printf "Id[%s] is missing\n" "${mID}";
            #printf "Id [%s] not present \n" "${mID}";
            # Print in parameter id table
            printAccess "${sFile}" "${mID}" "NA" "NA" "IMM" "REF_N" 0 0 0;
        fi
        #printf "Param[%s]\n" "${mID}";

    done;

    idMax_dec=$(echo "obase=16; ibase=10; ${offset};" | bc );
    
    printf "Replace in %s the \"UndefSizeForNow\" with 0x%s\n" "${hFile}" "${idMax_dec}";
    sed -i "s/UndefSizeForNow/${idMax_dec}/g" ${hFile};
    
    printIDs "${hFile}" "${mID}" "LAST_ID" "Don't remove, it marks the end of table.";
    
    # print header and source foot
    printf "};\n" >> ${sFile};
    
    printf "}param_ids_e;\n" >> ${hFile};
    
    printf "};\n" >> ${vFile};
    
    if [[ ${generate_setup} == "1" ]]
    then
        printf "};\n" >> ${tFile};
    fi
}

#===============================================================================

# Print one line of restriction table (in the source file)
# in : the source file name
# in : the restriction id
# in : the restriction type
# in : the restriction size
# in : the restriction nb
# in : the restriction reference table (where restrictions values are stored)
function printRestr {
    local sFile=$1;
    local mID=$2;
    local type=$3;
    local size=$4;
    local nb=$5;
    local ref_table=$6;
    printf "    INIT_RESTR_TABLE(0x%s, %s, %s, %d, %s),\n" ${mID} ${type} ${size} ${nb} ${ref_table} >> ${sFile};
}

# Build the Restrictions tables :
# - Restriction table 
# - Restriction Value table
# - Table defintion 
# in : the xml file 
# in : the restriction file 
# in : the restriction tables of values file
# in : the restriction tables header file
function buildRestrictionsTables {
    local mFile=$1;
    local sFile=$2;
    local vFile=$3;
    local hFile=$4;
    
    ########################################################################### 
    
    local field="Restriction";
    
    # get sorted id   
    getSortedID "${mFile}" "${field}";
    pID=(${gSortedID[@]});
    
    # get nb id and id max
    idNb=$(( ${#pID[@]} ))
    idMax=${pID[$(( ${idNb} -1 ))]} 
    
    # get nb id and id max as decimal number
    idNb_dec=$(echo "obase=10; ibase=16; ${#pID[@]};" | bc );   
    idMax_dec=$(echo "obase=10; ibase=16; ${pID[$(( ${idNb} -1 ))]};" | bc );
        
    # print restr file header    
    szMax=$(echo "obase=16; ibase=10; $(( ${idMax_dec} +1));" | bc );
    printf "\n/******************************************************************************/\n" >> ${sFile};
    printf "const restr_s a_ParamRestr[0x%s] = {\n" "${szMax}" >> ${sFile};

    # print header file header
    printf "\n/******************************************************************************/\n" >> ${hFile};
    printf "#define PARAM_RESTR_CFG_SZ (0x%s)\n\n" "${szMax}" >> ${hFile};
    printf "extern const restr_s a_ParamRestr[PARAM_RESTR_CFG_SZ];\n\n" >> ${hFile};
    
    # print value file header    
    printf "\n/******************************************************************************/\n" >> ${vFile};

    local i=0;
    local j=0;
    local offset=0;
    local mID=0;
    for j in $(seq 0 ${idMax_dec})
    do       
        pID_dec=$(echo "obase=10; ibase=16; ${pID[$i]};" | bc )
        
        #echo "j=$j; i=$i; pID_dec=${pID_dec}; pID[$i]=${pID[$i]}"
        if [[ $j == ${pID_dec} ]]
        then
            getAttr "${mFile}" "${field}" "rtype" ${pID[$i]}; rtype=${gParam};
            getAttr "${mFile}" "${field}" "rsize" ${pID[$i]}; rsize=${gParam};
            
            mID=${pID[$i]};
            
            # Get the parameter value 
            var=`${bCMD} "//${field}[@id=\"${pID[$i]}\"]" -n ${mFile}`
            value=$(echo ${var}|tr -d '\n');
            
            # Convert in array
            arr=(${value});
            nb=${#arr[@]};            
            
            # Build table name
            table_name="";
            case ${rtype} in 
                "RANGE")
                    table_name="a_"${rtype}"_"${mID};
                    ;;
                "ENUM")
                    table_name="a_"${rtype}"_"${mID};
                    ;;
                "MODULO")
                    table_name="a_"${rtype}"_"${mID};
                    ;;
                *)
                    table_name="a_UNDEF_"${mID};
                    ;;                    
            esac

            # Build table type
            table_type="const ";
            case ${rsize} in 
                "8BITS")
                    table_type+="uint8_t";
                    ;;
                "16BITS")
                    table_type+="uint16_t";
                    ;;
                "32BITS")
                    table_type+="uint32_t";
                    ;;
                "64BITS")
                    table_type+="uint64_t";
                    ;;
                *)
                    table_type+="undef_t";
                    ;;                    
            esac
            # print the header definition
            printf "extern %s %s[%d];\n" "${table_type}" "${table_name}" "${nb}" >> ${hFile};
            
            # Print the restriction values table
            printf "%s %s[%d] = { " "${table_type}" "${table_name}" "${nb}" >> ${vFile};
            for s in "${arr[@]}"; do 
                printf "%s, " "$s"  >> ${vFile};
            done
            printf "};\n" >> ${vFile};
            
            # Print in restriction table
            printRestr "${sFile}" "${mID}" "RESTR_${rtype}" "RESTR_${rsize}" "${nb}" "${table_name}";

            # Next parameter id
            i=$(($i+1));           
        else
            printf "Id[%s] is missing\n" "${mID}";
        fi
        #printf "Restr[%s]\n" "${mID}";
       
    done;
    #printIDs "${hFile}" "${mID}" "LAST_ID" "Don't remove, it marks the end of table.";
    
    # print header and source foot
    printf "};\n" >> ${sFile};
}

#===============================================================================
function usage {
    echo "Usage is :"
    echo " --in xml_file"
    echo " --dest dest_dir (Optional, default is :.)"
}

################################################################################
#

# Clean input merged xml file
function clean_merged_xml() {
    inFile=$1;
    mList=($( grep -n "<ParameterList>" ${inFile} | sed 's/:\s*<ParameterList>\s*//g' )); 
    unset 'mList[0]';
    for l in ${mList[@]}
    do 
        sed -i "${l}s/<ParameterList>/<\!-- Start -->/g" ${inFile}; 
    done;
    mList=($( grep -n "</ParameterList>" ${inFile} | sed 's/:\s*<\/ParameterList>\s*//g' )); 
    unset 'mList[-1]';
    for l in ${mList[@]}
    do 
        sed -i "${l}s/<\/ParameterList>/<\!-- End -->/g" ${inFile};
    done;
}

# header and source file name
header_FileName="parameters_cfg.h";
source_FileName="parameters_cfg.c";
value_FileName="parameters_default.c";
setup_FileName="parameters_setup.c";
generate_setup=0;

function build_tables(){

    local tag=$(stat ${xmlFile} | grep "Modif");
    printf "tag => %s\n" "${tag}";

    ###########################################################################
    # Start:Header file build
    headerFile="${DEST_PATH}/gen/${header_FileName}";
    
    # header definitions
    local header_str_head="
#ifndef _PARAMETERS_ID_H_
#define _PARAMETERS_ID_H_
#ifdef __cplusplus
extern "C" {
#endif

#include \"parameters_def.h\""

    local header_str_foot="
#ifdef __cplusplus
}
#endif
#endif /* _PARAMETERS_ID_H_ */";
    printHeaders "${xmlFile}(${tag})" "${headerFile}";
    printf "%s\n" "${header_str_head}">> ${headerFile};
    
    ###########################################################################
    # Start:Source file build
    sourceFile="${DEST_PATH}/gen/${source_FileName}";
    
    # source definition    
    local source_str_head="#include \"parameters_def.h\"";
    local source_str_foot="";    
    printHeaders "${xmlFile}(${tag})" "${sourceFile}";
    printf "%s\n" "${source_str_head}" >> ${sourceFile};
    printf "#include \"%s\"\n\n" "${header_FileName}" >> ${sourceFile};
    
    ###########################################################################
    # Start:Value file build
    valueFile="${DEST_PATH}/gen/${value_FileName}";
    
    # value definitions
    local value_str_head="#include \"parameters_def.h\""
    local value_str_foot=""
    printHeaders "${xmlFile}(${tag})" "${valueFile}";
    printf "%s\n\n" "${value_str_head}" >> ${valueFile};

    ###########################################################################
    # Start:Setup file build
    setupFile="${DEST_PATH}/gen/${setup_FileName}";
    # Generate setup file
    if [[ ${generate_setup} == "1" ]]
    then
        printHeaders "${xmlFile}(${tag})" "${setupFile}";
        printf "%s\n\n" "${value_str_head}" >> ${setupFile};
    fi
    ###########################################################################
    # Access tables build    
    buildParametersTables "${xmlFile}" "${sourceFile}" "${valueFile}" "${headerFile}" "${setupFile}";
    
    ###########################################################################
    # Restriction tables build
    buildRestrictionsTables "${xmlFile}" "${sourceFile}" "${valueFile}" "${headerFile}";
    
    ###########################################################################
    # End:Header file build
    printFooter "${headerFile}" "${header_str_foot}";
    
    ###########################################################################
    # End:Source file build    
    printFooter "${sourceFile}" "${source_str_foot}";
    
    ###########################################################################
    # End:Value file build    
    printFooter "${valueFile}" "${value_str_foot}";

    ###########################################################################
    # End:Setup file build    
    if [[ ${generate_setup} == "1" ]]
    then
        printFooter "${setupFile}" "${value_str_foot}";
    fi
    ###########################################################################
}

# 
is_silent=1;
if [[ ! ( -z ${IS_VERBOSE_ENV} ) && ${IS_VERBOSE_ENV} == 1 ]]
then
    is_silent=0;
fi


#
DEST_PATH=".";

pFile="parameters.xsd";
base_path=$(dirname $0);

xFile=${base_path}/"${pFile}";

################################################################################
#mFile="DefaultParams_LAN.xml"
xmlFile="DefaultParams_test.xml"

while [ $# -gt 0 ]
do
case $1 in
    --in)
        shift; INPUT_FILE="$1"; shift;
        ;;
    --dest)
        shift; DEST_PATH="$1"; shift;
        ;;
    --setup)
        generate_setup=1;
        shift;
        ;;
    *)
        shift;;
esac
done;  

if [[ ${INPUT_FILE} == "" ]]
then
    echo "You have to give an xml input file!!";
    usage;
else 
    xmlFile="${INPUT_FILE}";
    # clean imput xml file (after merged)
    echo "      -> Cleaning imput xml file (after merged)";
    clean_merged_xml ${xmlFile};
    
    # check input xml is conformed to the xsd file
    echo "      -> Check if input xml is conformed to the xsd file";
    xmlstarlet val -e -q --xsd ${xFile} ${xmlFile}
    RES=$?
    
    if [[ ${RES} != 0 ]]
    then
        echo ""
        echo "Error : the \"${xmlFile}\" file doesn't conform to \"${xFile}\"!!!"
        exit 0;
    else 
        mkdir -p "${DEST_PATH}/gen";
        build_tables;
    fi
fi

