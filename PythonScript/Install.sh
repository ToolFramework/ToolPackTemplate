#!/bin/bash
# This script installs the necessary dependencies for running python scripts within ToolFramework
CURDIR=${PWD}
cd "$( dirname "${BASH_SOURCE[0]}" )"
ToolAppPath="${PWD}/../../../.."
mkdir -p ${ToolAppPath}/Dependencies/cppyy
cd ${ToolAppPath}/Dependencies/cppyy
echo "Installing cppyy dependencies into ${PWD}"

echo "installing dependencies"
yum install gcc-c++ gcc make cmake git python36 python36-libs python36-devel python36-pip patch which
echo "upgrading python3 pip"
python3 -m pip install --upgrade pip

echo "setting envrionmental variables"
export STDCXX=11
export USER=`whoami`
export PATH=/home/${USER}/.local/bin:$PATH
export CLING_REBUID_PCH=1

echo "installing cppyy backend"
git clone https://github.com/wlav/cppyy-backend.git
cd cppyy-backend
git checkout cppyy-cling-6.25.2
cd cling
python3 setup.py egg_info
python3 create_src_directory.py
echo "building cling, this may take a while..."
python3 -m pip install . --upgrade

echo "building clingwrapper"
cd ../clingwrapper
python3 -m pip install . --upgrade --no-use-pep517 --no-deps

cd ../../
echo "building CPyCppyy interface"
git clone https://github.com/wlav/CPyCppyy.git
cd CPyCppyy
python3 -m pip install . --upgrade --no-use-pep517 --no-deps

echo "building cppyy"
cd ../
git clone https://github.com/wlav/cppyy.git
cd cppyy
python3 -m pip install . --upgrade --no-deps

echo "Complete!"
cd ${CURDIR}

return 0

