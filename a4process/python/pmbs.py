#!/usr/bin/env python

import fcntl
import thread
import time
import os
import random
import subprocess

color = {}  
color['normal'] = os.popen("tput sgr0").read()  
color['darkred'] = os.popen("tput setaf 1").read()  
color['green'] = os.popen("tput setaf 2").read()  
color['orange'] = os.popen("tput setaf 3").read()  
color['blue'] = os.popen("tput setaf 4").read()  
color['pink'] = os.popen("tput setaf 5").read()  
color['turquoise'] = os.popen("tput setaf 5").read()  
color['white'] = os.popen("tput setaf 7").read()  
color['red'] = os.popen("tput setaf 8").read()

class PMBSJob(object):
    def __init__(self,command):
        self.debug = False
        self.cmd = command
        self.id = 0
        self.process = None
        self.status = "new"

def get_slots_from_pod():
    os.system("/bin/bash -c '. /project/etpsw/Common/PoD/setup.sh; lproofnodes.sh 20'")
    slots_pod = [l.split(",") for l in file("pod_ssh.cfg").readlines()]
    slots = []
    for n, host, x, tmp, nw in slots_pod:
        host = host.strip()
        for n in xrange(int(nw)):
            slots.append(host)
    return slots

qlock   = thread.allocate_lock()
filter = []
jobs    = []
# Uncomment this for manual selection
slots   = [l.strip() for l in file(os.getenv("HOME")+os.sep+".pmbs").readlines() if l.strip() and not l.strip().startswith("#")]
#slots = get_slots_from_pod()
slotmap = [None]*len(slots)

# 'External' function
def submit(cmd, debug=False):
   j = PMBSJob(cmd)
   j.debug = debug
   qlock.acquire()
   jobs.append(j)
   qlock.release()
   return j

import fcntl, os

def printfilter(o,e,j):
   if len(o.strip()) > 0:
      for l in o.strip().split("\n"):
         filtered = False
         for f in filter:
            if f in l:
               filtered = True
         if not filtered:
            print "JOB %3i:" % (j.id), l
   if len(e.strip()) > 0:
      for l in e.strip().split("\n"):
         filtered = False
         for f in filter:
            if f in l:
               filtered = True
         if not filtered:
            print "JOB %3i:%s" % (j.id, color["red"]), l, color["normal"]

def PMBSMainLoop():
  jobid = 0
  runningjobs = []
  firstdelay = 0.1
  while True:
   freeslots = [i for i in range(0,len(slots)) if slotmap[i] == None]
   random.shuffle(freeslots)
   qlock.acquire()
   nextjobs = [j for j in jobs if j.status == "new"]
   qlock.release()
   nextjobs.reverse()
   #print "%s free slots for %s jobs..." % (len(freeslots),len(nextjobs))
   for slotn in freeslots:
      if len(nextjobs) > 0:
         j = nextjobs.pop()
         j.status = "running"
         slotmap[slotn] = j
         #j.pid = os.spawnvp(os.P_NOWAIT, "ssh", ["ssh","-x",slots[slotn],j.cmd])
         host = slots[slotn]
         j.id = jobid
         jobid += 1
         cmd = str(j.cmd)
         cmd = cmd.replace("{JOBID}", str(j.id))
         cmd = cmd.replace("{HOST}", str(host))
         if j.debug:
            j.process = subprocess.Popen(["echo", cmd], bufsize=0, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
         else:
            j.process = subprocess.Popen(["ssh", "-x", str(host), cmd], bufsize=0, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
         fcntl.fcntl(j.process.stdout, fcntl.F_SETFL, os.O_NONBLOCK)
         fcntl.fcntl(j.process.stderr, fcntl.F_SETFL, os.O_NONBLOCK) 
         runningjobs.append(j)
         print "Submitted job %s to %s" % (j.id, host)
         #print "Submitted job %s to %s to do %s" % (j.id, host, j.cmd % n)
         if j.id == 0:
            time.sleep(firstdelay)
         print "Remaining jobs: %s" % (len(nextjobs))
         time.sleep(0.1)
   
   for j in runningjobs:
      o = ""
      e = ""
      try:
         o = j.process.stdout.read()
      except IOError:
         pass
      try:
         e = j.process.stderr.read()
      except IOError:
         pass
      printfilter(o,e,j)

      if not j.process.poll() == None:
         o,e = j.process.communicate()
         printfilter(o,e,j)
         runningjobs.remove(j)
         slotmap[[i for i in range(0,len(slots)) if slotmap[i] == j][0]] = None
         if j.process.poll() == 0:
            j.status="completed"
            print "Job %s completed" % j.id
         else:
            j.status="failed"
            print "Job %s failed with status %s" % (j.id, j.process.poll())
         break

   time.sleep(0.05) 

def flush():
   cnt = 10
   while cnt > 0:
      qlock.acquire()
      cnt = len([j for j in jobs if j.status in ["new","running"]])
      qlock.release()
      time.sleep(2)
   print "PMBS done %i jobs succeeded, %i failed." % (len([j for j in jobs if j.status in ["completed"]]), len([j for j in jobs if j.status in ["failed"]]))

pbmsthread = thread.start_new(PMBSMainLoop, ())

if __name__=="__main__":
    import sys, optparse, math
    parser = optparse.OptionParser()
    parser.add_option("-c", "--command", default="echo I am {JOBID} on {HOST}!", help="The command to execute", metavar="CMD")
    parser.add_option("-f", "--filelist", default=None, help="file with files to process", metavar="FILE")
    parser.add_option("-n", "--number", default=1, help="number of files from filelist to append to each command")
    parser.add_option("-d", "--debug", action="store_true", help="Just do debug print of commands")
    (options, args) = parser.parse_args()

    if options.filelist is None:
        parser.print_help()
        sys.exit(-1)

    files = [f.strip() for f in file(options.filelist).readlines() if f.strip() and not f.strip().startswith("#")]
    n_files = len(files)
    n_chunks = int(math.ceil(n_files * 1.0 / int(options.number)))
    fpc = int(math.ceil(n_files/n_chunks))
    print files[0]

    chunks = []
    while files:
        chunk = []
        try:
            for x in xrange(fpc):
                chunk.append(files.pop())
        except IndexError:
            pass
        chunks.append(chunk)
    assert sum(map(len, chunks)) == n_files

    for c in chunks:
        j = submit(options.command + " " + " ".join(c), debug=options.debug)
    flush()
        

