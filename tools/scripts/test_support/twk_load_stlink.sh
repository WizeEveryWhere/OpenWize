#!/bin/bash 

# Default options
PROG_TOOL=`which STM32_Programmer_CLI`;
APP_BIN="";
START_ADDRESS=0x08000000
cmd="STM32_Programmer_CLI -c port=SWD mode=UR reset=HWrst";

function show_default(){
    echo "Main command is :";
    echo "   ${cmd}";
    echo "";
    echo "Download at :";
    echo "   ${START_ADDRESS}";
    echo "";
    echo "You should also be able to use it directely..." ;
    echo "";
    echo "";
    echo "The following will reset the probe 1";
    echo "STM32_Programmer_CLI -c port=SWD reset=HWrst index=${PROBE_INDEX}";
    echo "";
}

function usage(){
    echo "Usage :"
    echo "$(basename $0) prog_name.bin probe_id";
    echo "STM32_Programmer_CLI -l ; to get the probe_id" 
}

function do_it(){
    local idx=$1;
    local appbin=$2;
    ${cmd} index=$idx -d ${appbin} ${START_ADDRESS};
    sleep 1;
    ${cmd} index=$idx -hardRst;
}

if [[ "${PROG_TOOL}" == "" ]]
then
    echo "Error! It seems that the \"STM32_Programmer_CLI\" program doesn't exist.";
    echo "Ensure your set environement variable (see the main \"README.md\" and/or the \"set_env.sh\" script.";
    exit;
fi


APP_BIN=$1;
if [[ "${APP_BIN}" == "" ]]
then 
    echo "Error! Give an application file (binary) to download on the board.";
    usage;
    exit;
fi

PROBE_INDEX=$2
if [[ "${PROBE_INDEX}" == "" ]]
then 
    echo "Error! Give a probe index to be able to continu.";
    echo "Available devices :"
    ${PROG_TOOL} -l #st-link
    exit;
fi

show_default;
do_it "${PROBE_INDEX}" "${APP_BIN}";
