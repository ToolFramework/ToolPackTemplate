#set -x
CURDIR=${PWD}
thisdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Calling installer script for dependencies"
${thisdir}/Install.sh
if [ $? -ne 0 ]; then
    echo "Install script returned an error, aborting"
    return -1
fi

ToolAppPath="${thisdir}/../../../.."
ToolAppPath=$(readlink -f ${ToolAppPath})
echo "ToolAppPath is ${ToolAppPath}"

# provide a template python tool for users to use.
ln -s ${PWD}/Templates/PyTool.py ${ToolAppPath}/UserTools/template/

# add a script for creating new Tools based on it
ln -s ${PWD}/Templates/newPyTool.sh ${ToolAppPath}/UserTools/

# the Python API (Python.h) contains a lot of instances of 'long long', which
# technically is not part of the c++ standard. With the '-pedantic' flag, g++
# will output a slew of warnings to this effect, so let's silence them.
sed -i '/^CXXFLAGS=.*/a CXXFLAGS += -Wno-long-long' ${ToolAppPath}/Makefile

# add python includes and libraries to the Makefile
sed -i '/MyToolsInclude =.*/a MyToolsInclude += `python3-config --cflags`' ${ToolAppPath}/Makefile
sed -i '/MyToolsLib =.*/a MyToolsLib += `python3-config --libs`' ${ToolAppPath}/Makefile

# we need to add all UserTools directories to the PYTHONPATH environmental
# variable so that python tools will be found
cat << "EOF" >> ${ToolAppPath}/Setup.sh

for folder in `ls -d ${PWD}/UserTools/*/ `
do
    export PYTHONPATH=$folder:${PYTHONPATH}
done
EOF

