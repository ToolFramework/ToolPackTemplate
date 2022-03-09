#!/bin/bash
#set -x
#set -e
# sanity checks
# check we're root (required for installing dependencies)
if [ "$(whoami)" != "root" ]; then
	echo "This script must be run as root"
	exit 1
fi

# check we have a non-root user for installing pip dependencies
TOOLUSER="tool"
USERINVALID=`id -u ${TOOLUSER} &> /dev/null; echo $?`
while [ ${USERINVALID} -ne 0 ]; do
	echo "user 'tool' not found"
	echo "do you wish to install cppyy in another user's directory?"
	select result in Yes No; do
		if [ "$result" == "Yes" ] || [ "$result" == "No" ]; then
			if [ "$result" == "No" ]; then
				echo "terminating.";
				exit 1;
			else
				echo "enter the user whose home directory you would like to install cppyy into"
				read TOOLUSER
				USERINVALID=`id -u ${TOOLUSER} &> /dev/null; echo $?`
			fi
		else
			echo "please enter 1 or 2";
		fi
	done
done
echo "installing cppyy for user ${TOOLUSER}"

CURDIR=${PWD}
cd "$( dirname "${BASH_SOURCE[0]}" )"

# check the ToolFramework folder exists and is writable
#TOOLFRAMEWORKDIR="/opt/ToolFrameworkCore"
TOOLFRAMEWORKDIR=$(readlink -f ${PWD}/../../../..)
if [ ! -d ${TOOLFRAMEWORKDIR} ] || [ ! -w ${TOOLFRAMEWORKDIR} ]; then
	# doesn't exist or isn't writable (perhaps an immutable container?)
	echo "ToolFramework directory ${TOOLFRAMEWORKDIR} is not present or is not writable!"
	exit 1
fi
DEPENDENCIESDIR=${TOOLFRAMEWORKDIR}/Dependencies
# make dependencies dir it if it doesn't exist... is this sensible?
mkdir -p ${DEPENDENCIESDIR}

# make non-root user home directory (installation dir of pip installs), if it doesn't exist
if [ ! -d /home/${TOOLUSER} ]; then
	echo "making home directory for user ${TOOLUSER}"
	mkdir -p /home/${TOOLUSER}
	chown -R `id -u ${TOOLUSER}` /home/${TOOLUSER}
fi

echo "installing dependencies"
yum install -y gcc-c++ gcc make cmake git python36 python36-libs python36-devel python36-pip patch which sudo

# upgrade pip (required for --no-use-pep517 flag)
echo "updating pip"
sudo -u ${TOOLUSER} python3 -m pip install --user --upgrade pip
# we'll also need wheel, or the CPyCppyy install will fail
sudo -u ${TOOLUSER} python3 -m pip install wheel

# setup installation environment
cd ${DEPENDENCIESDIR}
export STDCXX=11
#export EXTRA_CLING_ARGS='-nocudainc'
export CLING_REBUID_PCH=1
export PATH=/home/${TOOLUSER}/.local/bin:$PATH
#export LD_LIBRARY_PATH=/home/${TOOLUSER}/.local/lib/python3.6/site-packages/cppyy_backend/lib:$LD_LIBRARY_PATH

# start installation
echo "installing cppyy_backend"
mkdir cppyy-backend
chmod -R 777 cppyy-backend/
sudo -u ${TOOLUSER} git clone https://github.com/wlav/cppyy-backend.git cppyy-backend/
cd cppyy-backend
sudo -u ${TOOLUSER} git checkout cppyy-cling-6.25.2
cd cling
sudo -u ${TOOLUSER} -E python3 setup.py egg_info
sudo -u ${TOOLUSER} -E python3 create_src_directory.py
sudo -u ${TOOLUSER} -E python3 -m pip install . --user --upgrade
cd ../clingwrapper
sudo -u ${TOOLUSER} -E python3 -m pip install . --user --upgrade --no-use-pep517 --no-deps

echo "installing CPyCppyy"
cd ../../
mkdir CPyCppyy
chmod -R 777 CPyCppyy
sudo -u ${TOOLUSER} git clone https://github.com/wlav/CPyCppyy.git CPyCppyy
cd CPyCppyy
git checkout CPyCppyy-1.12.8
sudo -u ${TOOLUSER} -E python3 -m pip install . --user --upgrade --no-use-pep517 --no-deps

echo "installing cppyy"
cd ../
mkdir cppyy
chmod -R 777 cppyy
sudo -u ${TOOLUSER} git clone https://github.com/wlav/cppyy.git cppyy
cd cppyy
sudo -u ${TOOLUSER} -E python3 -m pip install . --user --upgrade --no-deps --verbose

echo "adding /home/${TOOLUSER}/.local/lib/python3.6/site-packages to PYTHONPATH"
export PYTHONPATH=/home/${TOOLUSER}/.local/lib/python3.6/site-packages:$PYTHONPATH

echo "updating cppyy precompiled header"
rm /home/${TOOLUSER}/.local/lib/python3.6/site-packages/cppyy/allDict.cxx.pch.*

# a quick test, also trigger rebuilding of the pch
echo "the following test should print 1 2 3 4 5 6 7 8 9"
RESULT=$(cat << EOF | python3
import cppyy
from cppyy.gbl.std import vector
v = vector[int](range(10))
for m in v: print(m, end=' ')

EOF
)
# we need to do it twice as the first time it prints out a message about recompiling the header
RESULT=$(cat << EOF | python3
import cppyy
from cppyy.gbl.std import vector
v = vector[int](range(10))
for m in v: print(m, end=' ')

EOF
)

echo $RESULT
if [ "$RESULT" != "0 1 2 3 4 5 6 7 8 9" ]; then
	echo "Test Failed! Check your installation of python3 and cppyy!"
	exit 1
else
	echo "Test Passed"
fi

# add the path to cppyy module to the Setup.sh
cat << "EOF" >> ${TOOLFRAMEWORKDIR}/Setup.sh

export PYTHONPATH=/home/${TOOLUSER}/.local/lib/python3.6/site-packages:$PYTHONPATH

EOF

echo "Installation complete"
exit 0
