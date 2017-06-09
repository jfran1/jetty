#!/bin/bash

module load gcc
module load ROOT

module list

function abspath()
{
  case "${1}" in
    [./]*)
    echo "$(cd ${1%/*}; pwd)/${1##*/}"
    ;;
    *)
    echo "${PWD}/${1}"
    ;;
  esac
}

savedir=$PWD

THISFILE=`abspath $BASH_SOURCE`
XDIR=`dirname $THISFILE`
if [ -L ${THISFILE} ];
then
    target=`readlink $THISFILE`
    XDIR=`dirname $target`
fi

THISDIR=$XDIR
XDIR=`dirname $XDIR`

export JETTYDIR=$XDIR

if [ -z $PATH ]; then
	export PATH=$JETTYDIR/bin:$JETTYDIR/scripts:$HOME/pythia8226/bin
else
	export PATH=$JETTYDIR/bin:$JETTYDIR/scripts:$HOME/pythia8226/bin:$PATH
fi

if [ -z $LD_LIBRARY_PATH ]; then
    export LD_LIBRARY_PATH=$JETTYDIR/lib:$HOME/pythia8226/lib
else
    export LD_LIBRARY_PATH=$JETTYDIR/lib:$HOME/pythia8226/lib:$LD_LIBRARY_PATH
fi
