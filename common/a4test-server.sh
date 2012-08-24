#! /usr/bin/env bash

export UTARGET=/home/sirius/www/www.ebke.org/a4
export DTARGET=/home/sirius/www/www.ebke.org/a4-dev

if hostname | grep -q lxplus; then
    # Needed so that configure finds the right python binary
    export PYTHON=python26
    #source /afs/cern.ch/sw/lcg/external/gcc/4.3.2/x86_64-slc5/setup.sh
    #source /afs/cern.ch/sw/lcg/external/gcc/4.4.3/x86_64-slc5-gcc44-opt/setup.sh
    source /afs/cern.ch/sw/lcg/external/gcc/4.5.0/x86_64-slc5-gcc43-opt/setup.sh
    #source /afs/cern.ch/sw/lcg/external/gcc/4.6.1/x86_64-slc5/setup.sh
    
    source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.30.02/x86_64-slc5-gcc43-opt/root/bin/thisroot.sh
else
    source ~/Software/root532/bin/thisroot.sh
fi

# Do this _AFTER_ the above otherwise the setup scripts give us errors
set -u -e

GHNAME=${1}
BRANCH=${2}

N_CPUS=$(cat /proc/cpuinfo | grep processor | wc -l)
# Use half of the machine's capacity
N_PARALLEL=$(($N_CPUS / 2))

if [[ -z "${3-}" ]]; then
    TEMPDIR=$(mktemp -d)
else
    echo "Using commandline TEMPDIR=${3}"
    TEMPDIR="${3}"
fi


TIME="/usr/bin/time -o ${TEMPDIR}/time_"

function recordtime {
    NAME="${1}"
    shift
    /usr/bin/time -o ${TEMPDIR}/time_${NAME} $@
}

function cleanup {
    chmod -R +w $TEMPDIR
    rm -Rf $TEMPDIR
}
trap cleanup 0

pushd $TEMPDIR &> /dev/null

# do not abort if the next command fails
set +u +e


(
    # abort subshell if something fails
    set -u -e


    CACERT=${HOME}/a4/common/github-ca/curl-ca-bundle-github.crt
    if ! [[ -e a4.tar.gz ]]; then
        recordtime "wget" wget --ca-certificate ${CACERT} https://github.com/$GHNAME/a4/tarball/$BRANCH -O a4.tar.gz
    else
        echo "Already downloaded a4, continuing"
    fi

    mkdir -p a4
    
    # Pipe through tail so we don't prematurely close the pipe
    recordtime "untar" tar xf a4.tar.gz -C a4 --strip-components 1
    
    pushd a4 &> /dev/null
    
    ls -l
    
    recordtime "build_boost"    ./get_miniboost.sh
    recordtime "build_protobuf" ./get_protobuf.sh
    recordtime "build_snappy"   ./get_snappy.sh
    
    recordtime "configure" ./waf configure --prefix=$(pwd)
    
    recordtime "build_j${N_PARALLEL}" ./waf build -j${N_PARALLEL}
    
    recordtime "check" ./waf --checkall --valgrind

    recordtime "installcheck" ./waf install --checkall --valgrind
    
    recordtime "distcheck" ./waf distcheck

    if [[ $GHNAME == "JohannesEbke" ]]; then
        recordtime "doxygen" ./waf doxygen 2>&1
        rm -rf $UTARGET $DTARGET
        mv build/doc/user/html $UTARGET
        mv build/doc/dev/html $DTARGET
    fi;
    
    popd

) &> ${TEMPDIR}/log

RESULT=$?

popd &> /dev/null

if [[ $RESULT != 0 ]]; then
    echo "The compilation of $GNAME/a4/$BRANCH failed." > ${TEMPDIR}/mail
    echo >> ${TEMPDIR}/mail
    echo "The last lines of output are:" >> ${TEMPDIR}/mail
    echo "--------------------------------------------------------------------------------" >> ${TEMPDIR}/mail
    tail -n20 ${TEMPDIR}/log >> ${TEMPDIR}/mail
    echo "--------------------------------------------------------------------------------" >> ${TEMPDIR}/mail
    echo >> ${TEMPDIR}/mail
    echo "Full output:" >> ${TEMPDIR}/mail
    echo "--------------------------------------------------------------------------------" >> ${TEMPDIR}/mail
    cat ${TEMPDIR}/log >> ${TEMPDIR}/mail
    cat ${TEMPDIR}/mail | mail -s "a4test of $GHNAME/$BRANCH failed on ebke.org" sirius peter.waller@gmail.com
fi

# always exit with 0 so cron does not send us another mail
exit 0

