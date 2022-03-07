#!/bin/bash

#STM32CubeIde_ver=1.1.0
STM32CubeIde_ver=1.8.0


if [[ ${STM32CubeIde_ver} == '1.8.0' ]]
then
    ExplGnuPath="com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.9-2020-q2-update.linux64_2.0.0.202105311346";
    ExplProgPath="com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_2.0.100.202110141430";
else
    ExplGnuPath="com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.linux64_1.0.0.201904181610";
    ExplProgPath="com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_1.1.0.201910081157";
fi

ExplBasePath="/opt/Application/st/stm32cubeide_${STM32CubeIde_ver}/plugins"


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
function create_bashtools(){
    local _m_path=$1;
    local _m_cube_prog_path=$2;
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
export CROSS_TOOL_PATH=${_m_path}

# Use to find STM32CubeProgrammer
export CUBE_PROG_PATH=${_m_cube_prog_path}

# Set the PATH env. variable
export PATH=\$PATH:\${CROSS_TOOL_PATH}/bin:\${CUBE_PROG_PATH}/bin

EOF
    set_bashrc;
    echo "Done."
}

#-------------------------------------------------------------------------------
# Install the environment variable into ~/.bash_tools and ~/.bashrc
function install_env(){
    # cross-tools
    expl_path="${ExplBasePath}/${ExplGnuPath}/tools";
    
    echo "Set the cross-tools path : (expl: ${expl_path})";
    read -p "Path : " m_path;
    if [[ -z "${m_path}" ]]
    then
        echo "Cross-Tool path set to default : ${expl_path}";
        m_path=${expl_path};
    fi
    if [[ !(-d  ${m_path}) ]]
    then
        echo "The given path dosen't exist!!!";
        exit;
    fi
    echo "";
   
    # stmcube programmer
    expl_path="${ExplBasePath}/${ExplProgPath}/tools";
    
    echo "Set the STM32Cube programmer path : (expl: ${expl_path})";
    read -p "Path : " m_cube_prog_path;
    if [[ -z "${m_cube_prog_path}" ]]
    then
        echo "STM32Cube programmer path set to default : ${expl_path}";
        m_cube_prog_path=${expl_path};
    fi
    if [[ !(-d  ${m_cube_prog_path}) ]]
    then
        echo "The given path dosen't exist!!!";
        exit;
    fi
    echo "";
   
    #
    echo "Do you want to install in your environment variable (bashrc) (recommended)";
    read -p "Answer (yes or no): " m_answer;
    case ${m_answer} in
        [yYoO]*) 
            create_bashtools "${m_path}" "${m_cube_prog_path}";
            echo "...now, you can start a new shell, the variable will be set."
            ;;
        [nN]*) 
            export CROSS_TOOL_PATH=${m_path}
            export CUBE_PROG_PATH=${m_cube_prog_path}
            echo "";
            echo "You have to set the following variables for each new shell:";
            echo "export CROSS_TOOL_PATH=${m_path}";
            echo "export CUBE_PROG_PATH=${m_cube_prog_path}";
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

