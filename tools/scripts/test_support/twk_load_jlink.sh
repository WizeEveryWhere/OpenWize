#!/bin/bash 

# Default options
PROG_TOOL=JLinkExe
APP_BIN="../_install/bin/My_App.bin"
START_ADDRESS=0x8000000
SPECIAL_ADDRESS=0x08078000

if [[ -z "${DEVICE}" ]]
then
    DEVICE=STM32L451CE
else
    echo "Get environement DEVICE variable";
fi
ITF=SWD
SPEED=4000
echo "${DEVICE} device is selected";

TEMP_FILE=./temp_cmdFile.jlink

function build_ConnectFile()
{
cat <<- EOF > ${TEMP_FILE}
si ${ITF}
speed ${SPEED}
RSetType 0
EOF
}

function build_CmdFile()
{
cat <<- EOF >> ${TEMP_FILE}
loadbin ${APP_BIN}, ${START_ADDRESS}
verifybin ${APP_BIN}, ${START_ADDRESS}
r
g
EOF
}

function rm_CmdFile()
{
    rm -f ${TEMP_FILE};
    echo "";
}

function do_it()
{
    ${PROG_TOOL} -device ${DEVICE} -CommandFile ${TEMP_FILE};
    #${PROG_TOOL} -device ${DEVICE} -if ${ITF} -speed ${SPEED} -CommanderScript ${TEMP_FILE};   
}

function usage(){
    echo "Usage :"
    echo "$(basename $0) [option]"
    echo "  -p prog_name.bin : Program the board with prog_name.bin";
    echo "  -e : Erase the entire Flash Memory";
    echo "  -r : Reset the board";
    echo "  -s : Erase the special double-word at ${SPECIAL_ADDRESS}";
    echo "";
    echo "Optionaly env. variable : "
    echo "DEVICE : select the MCU device";
    echo "    WizeUp board : STM32L451CE (default)";
    echo "    TU board : STM32L451VE";
    echo "";
}


if [[ "$#" -ge 1 ]];
then
    case $1 in
    "-p")
        APP_BIN=$2;
        if [[ "${APP_BIN}" == "" ]]
        then 
            echo "Error! Give an application file (binary) to download on the board.";
            usage;
            exit;
        fi
        build_ConnectFile;
	echo "r" >> ${TEMP_FILE};
        build_CmdFile;
	echo "q" >> ${TEMP_FILE};
        do_it;
        rm_CmdFile;
        ;;
    "-e")
        build_ConnectFile;
	echo "r" >> ${TEMP_FILE};
	echo "erase" >> ${TEMP_FILE};
	echo "q" >> ${TEMP_FILE};
        do_it;
        rm_CmdFile;
        ;;

    "-s")
        build_ConnectFile;
	echo "w4 ${SPECIAL_ADDRESS}, 0xFFFFFFFF" >> ${TEMP_FILE};
	echo "q" >> ${TEMP_FILE};
        do_it;
        rm_CmdFile;
        ;;
    "-r")
        build_ConnectFile;
        echo "r" >> ${TEMP_FILE};
        echo "q" >> ${TEMP_FILE};
        do_it;
        rm_CmdFile;
        ;;
    "-rh")
        build_ConnectFile;
        echo "RSetType 2" >> ${TEMP_FILE};
	echo "r" >> ${TEMP_FILE};
        echo "q" >> ${TEMP_FILE};
        do_it;
        rm_CmdFile;
        ;;

    *)
        echo "Unknown command";
        usage;
        ;;
    esac
else
    echo "At least 1 parameters has to be given";
    usage;
fi
