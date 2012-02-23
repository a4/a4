source $(bindir)/this_a4.sh && $(pkgdatadir)/test_pyrw
source $(bindir)/this_a4.sh && a4info.py -m pytest_fw.a4 > /dev/null
source $(bindir)/this_a4.sh && a4info.py -m pytest_bw.a4 > /dev/null
source $(bindir)/this_a4.sh && a4dump.py -a pytest_fw.a4 > /dev/null
source $(bindir)/this_a4.sh && a4dump.py -a pytest_bw.a4 > /dev/null
./test_read pytest_fw.a4 > /dev/null
./test_read pytest_bw.a4 > /dev/null
./test_read pytest_fwfw.a4 > /dev/null
./test_read pytest_bwfw.a4 > /dev/null
./test_read pytest_fwbw.a4 > /dev/null
./test_read pytest_bwbw.a4 > /dev/null
./test_write_fw  
./test_write_bw 
./test_write_nomd_fw 
./test_write_nomd_bw 
./test_read test_fw.a4 
./test_read test_bw.a4 
./test_read_nomd test_nomd_fw.a4 
./test_read_nomd test_nomd_bw.a4
ln -s test_fw.a4 test_fw1.a4
ln -s test_bw.a4 test_bw1.a4
cat test_fw.a4 test_fw1.a4 > test_fwfw.a4 
cat test_bw.a4 test_bw1.a4 > test_bwbw.a4 
cat test_fw.a4 test_bw.a4 > test_fwbw.a4 
cat test_bw.a4 test_fw.a4 > test_bwfw.a4 
rm test_fw1.a4 test_bw1.a4
./test_read test_fwfw.a4 
./test_read test_fwbw.a4 
./test_read test_bwfw.a4 
./test_read test_bwbw.a4
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fw.a4",1000))'
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bw.a4",1000))'
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwfw.a4",2000))'
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_fwbw.a4",2000))'
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwfw.a4",2000))'
source $(bindir)/this_a4.sh && $(PYTHON) -c 'from a4.stream import test_read; import sys; sys.exit(test_read("test_bwbw.a4",2000))'
rm -f test_fw.a4 test_bw.a4 test_fwfw.a4 test_bwfw.a4 test_fwbw.a4 test_bwbw.a4
rm -f pytest_fw.a4 pytest_bw.a4 pytest_fwfw.a4 pytest_bwfw.a4 pytest_fwbw.a4 pytest_bwbw.a4
rm -f test_rw.a4 test_mm.a4
rm -f test_nomd_fw.a4 test_nomd_bw.a4
rm -f test_thread.a4 test_io.a4
