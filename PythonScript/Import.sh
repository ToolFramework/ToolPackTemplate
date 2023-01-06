#set -x
#set -e
CURDIR=${PWD}
thisdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Calling installer script for dependencies"
${thisdir}/Install.sh
if [ $? -ne 0 ]; then
    echo "Install script returned an error, aborting"
    exit 1
fi

ToolAppPath="${thisdir}/../../../.."
ToolAppPath=$(readlink -f ${ToolAppPath})
echo "ToolAppPath is ${ToolAppPath}"

# check if this tool has already been imported into the main repo
if [ -e ${ToolAppPath}/UserTools/template/PyTool.py ]; then
	# PythonScript tool already in parent repo
	exit 0
fi

# provide a template python tool for users to use.
ln -s ${thisdir}/Templates/PyTool.py ${ToolAppPath}/UserTools/template/

# add a script for creating new Tools based on it
ln -s ${thisdir}/Templates/newPyTool.sh ${ToolAppPath}/UserTools/
chmod +x ${thisdir}/Templates/newPyTool.sh

# the Python API (Python.h) contains a lot of instances of 'long long', which
# technically is not part of the c++ standard. With the '-pedantic' flag, g++
# will output a slew of warnings to this effect, so let's silence them.
sed -i '/^CXXFLAGS=.*/a CXXFLAGS += -Wno-long-long' ${ToolAppPath}/Makefile

# add python includes and libraries to the Makefile
sed -i '/MyToolsInclude =.*/a MyToolsInclude += `python3-config --cflags`' ${ToolAppPath}/Makefile
# for python3.8+ we need to add `--embed` to `--libs` or `--ldflags` to get `-lpython3.*`
# Using `--ldflags` includes the lib directory `-L/..` as well as the libraries `-l..`
LIBFLAGS=$(python3-config --ldflags --embed &>/dev/null && echo "python3-config --ldflags --embed" || echo "python3-config --ldflags")
LIBLINE='MyToolsLib += `'"${LIBFLAGS}"'`'
awk -i inplace -v "var=${LIBLINE}" '{print} /MyToolsLib/ && !x {print var; x=1}' ${ToolAppPath}/Makefile

# add the path to cppyy module to the Setup.sh
PACKAGEPATH=$(python3 -m pip show cppyy_backend | grep 'Location' | cut -d' ' -f 2)
cat << EOF >> ${TOOLFRAMEWORKDIR}/Setup.sh

export PYTHONPATH=${PACKAGEPATH}:\$PYTHONPATH

EOF

# we also need to add all UserTools directories to the PYTHONPATH
# so that python tools will be found
cat << "EOF" >> ${ToolAppPath}/Setup.sh

for folder in `ls -d ${PWD}/UserTools/*/ `
do
    export PYTHONPATH=$folder:${PYTHONPATH}
done
EOF

exit 0
