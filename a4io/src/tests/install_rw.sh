#!/bin/bash
set -e
set -u

mkdir -p testinstallrw
pushd testinstallrw

source ${BINDIR}/this_a4.sh
export PATH=$PATH:.:${SRCDIR}/a4io/src/tests

do_pyrw
a4info.py -m pytest_fw.a4 > /dev/null
a4info.py -m pytest_bw.a4 > /dev/null
a4dump.py -a pytest_fw.a4 > /dev/null
a4dump.py -a pytest_bw.a4 > /dev/null
../read pytest_fw.a4 > /dev/null
../read pytest_bw.a4 > /dev/null
../read pytest_fwfw.a4 > /dev/null
../read pytest_bwfw.a4 > /dev/null
../read pytest_fwbw.a4 > /dev/null
../read pytest_bwbw.a4 > /dev/null
../write_fw  
../write_bw 
../write_nomd_fw 
../write_nomd_bw 
../read test_fw.a4 
../read test_bw.a4 
../read_nomd test_nomd_fw.a4 
../read_nomd test_nomd_bw.a4
ln -s test_fw.a4 test_fw1.a4
ln -s test_bw.a4 test_bw1.a4
cat test_fw.a4 test_fw1.a4 > test_fwfw.a4 
cat test_bw.a4 test_bw1.a4 > test_bwbw.a4 
cat test_fw.a4 test_bw.a4 > test_fwbw.a4 
cat test_bw.a4 test_fw.a4 > test_bwfw.a4 
rm test_fw1.a4 test_bw1.a4
../read test_fwfw.a4 
../read test_fwbw.a4 
../read test_bwfw.a4 
../read test_bwbw.a4
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fw.a4",1000))'
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bw.a4",1000))'
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwfw.a4",2000))'
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwbw.a4",2000))'
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwfw.a4",2000))'
python -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwbw.a4",2000))'
rm -f test_fw.a4 test_bw.a4 test_fwfw.a4 test_bwfw.a4 test_fwbw.a4 test_bwbw.a4
rm -f pytest_fw.a4 pytest_bw.a4 pytest_fwfw.a4 pytest_bwfw.a4 pytest_fwbw.a4 pytest_bwbw.a4
rm -f test_rw.a4 test_mm.a4
rm -f test_nomd_fw.a4 test_nomd_bw.a4
rm -f test_thread.a4 test_io.a4

popd
