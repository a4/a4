#!/bin/bash
set -e
set -u

mkdir -p testrw
pushd testrw

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
../read test_fwfw.a4 
../read test_fwbw.a4 
../read test_bwfw.a4 
../read test_bwbw.a4

rm -f test_fw1.a4 test_bw1.a4
rm -f test_fw.a4 test_bw.a4 test_fwfw.a4 test_bwfw.a4 test_fwbw.a4 test_bwbw.a4
rm -f pytest_fw.a4 pytest_bw.a4 pytest_fwfw.a4 pytest_bwfw.a4 pytest_fwbw.a4 pytest_bwbw.a4
rm -f test_rw.a4 test_mm.a4
rm -f test_nomd_fw.a4 test_nomd_bw.a4
rm -f test_thread.a4 test_io.a4

popd testrw
