#!/bin/bash

#STM32CubeIde_ver=1.1.0
STM32CubeIde_ver=1.8.0

#--------------
ExplBasePath="/opt/Application/st/stm32cubeide_${STM32CubeIde_ver}/plugins";

#--------------
GnuTool_BasePath="com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32";
StProg_BasePath="com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer";
Jlink_BasePath="com.st.stm32cube.ide.mcu.externaltools.jlink";

#--------------
GnuPath=$(find ${ExplBasePath} -type d -name "${GnuTool_BasePath}*");
StProgPath=$(find ${ExplBasePath} -type d -name "${StProg_BasePath}*");
JlinkPath=$(find ${ExplBasePath} -type d -name "${Jlink_BasePath}*");


#-------------------------------------------------------------------------------
# Save the ~/.bashrc into ~/.bashrc_save
function _save_bashrc(){
    local m_file=$1;
    echo "Saving ${m_file} into ${m_file}_save";
    cp ${m_file} ${m_file}_save;
}

#-------------------------------------------------------------------------------
# Restore ~/.bashrc from ~/.bashrc_save
function _restore_bashrc(){
    local m_file=$1;
    echo "Reload ${m_file}_save into ${m_file}"
    cp ${m_file}_save ${m_file}
}

#-------------------------------------------------------------------------------
# Modify the ~/.bashrc with a call to ~/.bash_tools
function set_bashrc(){
    local _m="$HOME";
    local m_file=${_m}/".bashrc"
    if [[ -e ${m_file} ]]
    then
        if [[ -e ${m_file}_save ]]
        then
            _restore_bashrc "${m_file}";
        else
            _save_bashrc "${m_file}";
        fi
    else 
        echo "${m_file} file dosen't exist!!"
        exit;
    fi
    echo "Modifying ${m_file}"
    
    echo "

# Add call to bash_tools
if [ -f ~/.bash_tools ]; then
    . ~/.bash_tools
fi
" >> ${m_file};
    echo "Done."
}

#-------------------------------------------------------------------------------
# Create the ~/.bash_tools script
function create_bashtools()
{
    local _m_path=$1;
    local _GnuPath=$2;
    local _StProgPath=$3;
    local _JlinkPath=$4;
    
    local _m="$HOME";
    local m_file=".bash_tools"
    
    
    echo "Creating ${_m}/${m_file}"
    if [[ -e ${_m}/${m_file} ]]
    then
        rm -f ${_m}/${m_file}
    fi
    now=`date`

cat << EOF > ${_m}/${m_file}
#!/bin/bash 
# bash_tools, generated the ${now}

# Use to find cross-tools
export CROSS_TOOL_PATH=${_GnuPath}

# Use to find STM32CubeProgrammer
export ST_PROG_PATH=${_StProgPath}

# Use to find Segger Jlink
export JLINK_PROG_PATH=${_JlinkPath}

# Set the PATH env. variable
export PATH=\$PATH:\${CROSS_TOOL_PATH}:\${ST_PROG_PATH}:\${JLINK_PROG_PATH};

EOF
    set_bashrc;
    echo "Done."
}

#-------------------------------------------------------------------------------
# Install the environment variable into ~/.bash_tools and ~/.bashrc
function install_env()
{
    #--------------------
    # stm32 main path
    expl_path="${ExplBasePath}";
    
    echo "Set the STM32Cube installation main path : (expl: ${expl_path})";
    read -p "Path : " m_path;
    if [[ -z "${m_path}" ]]
    then
        echo "STM32Cube path set to default : ${expl_path}";
        m_path=${expl_path};
    fi
    if [[ !(-d  ${m_path}) ]]
    then
        echo "The given path dosen't exist!!!";
        exit;
    fi
    echo "";
    
    #--------------------
    # Check if GNU tools are found
    GnuPath+=/tools/bin
    if [[ ! -e ${GnuPath} ]]
    then
        echo "${GnuPath}";
        echo "Was not found !!!!";
        echo "";
    else
        echo "Found : ${GnuPath}";
    fi

    #--------------------
    # Check if ST Programer tools are found
    StProgPath+=/tools/bin
    if [[ ! -e ${StProgPath} ]]
    then
        echo "${StProgPath}";
        echo "Was not found !!!!";
        echo "";
    else
        echo "Found : ${StProgPath}";
    fi
    
    #--------------------
    # Check if Segger JLink tools are found
    JlinkPath+=/tools/bin
    if [[ ! -e ${JlinkPath} ]]
    then
        echo "${JlinkPath}";
        echo "Was not found !!!!";
        echo "";
    else
        echo "Found : ${JlinkPath}";
    fi

    #--------------------
    echo "Do you want to install in your environment variable (bashrc) (recommended)";
    read -p "Answer (yes or no): " m_answer;
    case ${m_answer} in
        [yYoO]*) 
            create_bashtools "${m_path}" "${GnuPath}" "${StProgPath}" "${JlinkPath}";
            echo "...now, you can start a new shell, the variable will be set."
            ;;
        [nN]*) 
            export CROSS_TOOL_PATH=${GnuPath}
            export ST_PROG_PATH=${StProgPath}
            export JLINK_PROG_PATH=${JlinkPath}
            export PATH=$PATH:${CROSS_TOOL_PATH}:${ST_PROG_PATH}:${JLINK_PROG_PATH};
            echo "";
            echo "You have to set the following variables for each new shell:";
            echo "export CROSS_TOOL_PATH=${GnuPath}";
            echo "export ST_PROG_PATH=${StProgPath}";
            echo "export JLINK_PROG_PATH=${JlinkPath}";
            echo "";
            echo "export PATH=\$PATH:\${CROSS_TOOL_PATH}:\${ST_PROG_PATH}:\${JLINK_PROG_PATH}";
            echo "";
            
            ;;
        *) 
            echo "Answer is not correct.";;
    esac;
}

#-------------------------------------------------------------------------------
# Uninstall the environment variable into ~/.bashrc. The ~/.bash_tools is kept in place.
function uninstall_env(){
    local _m="$HOME";
    local m_file=${_m}/".bashrc"
    if [[ -e ${m_file}_save ]]
    then
        _restore_bashrc "${m_file}";
        rm -f ${m_file}_save
    else
        echo "...nothing to do";
    fi
}

#-------------------------------------------------------------------------------
# Usage/help; function
function usage(){
cat << EOF

$0 : Install/uninstall environment variable into bashrc.

Usage is : $0 option,
with option:
   -i install environment variable in bashrc.
   -u uninstall environment variable in bashrc.
   -h this help.

EOF
}


################################################################################
# The main/menu 
case $1 in 
    -i|--install)
        echo "Installing environment variable into bashrc"
        install_env;
        ;;
    -u|--uninstall)
        echo "Uninstalling environment variable into bashrc"
        uninstall_env;
        ;;
    -h|--help)
        usage;
        ;;
    *)
        echo "Error: Unknown option $1"
        usage;
        ;;
esac;

