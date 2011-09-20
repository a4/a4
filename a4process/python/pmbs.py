import thread, time, os, random, subprocess
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

filter = []

class PMBSJob(object):
   cmd = ""
   process = None
   status = "new"
   id = 0
   def __init__(self,command):
      self.cmd = command

qlock   = thread.allocate_lock()
jobs    = []

import os
os.system("/bin/bash -c '. /project/etpsw/Common/PoD/setup.sh; lproofnodes.sh 20'")

slots_pod = [l.split(",") for l in file("pod_ssh.cfg").readlines()]
slots = []
for n, host, x, tmp, nw in slots_pod:
    host = host.strip()
    slots.append((host, tmp, int(nw)))

#slots   = [l.strip() for l in file(os.getenv("HOME")+os.sep+".pmbs").readlines()]
slotmap = [None]*len(slots)

def submit(cmd):
   j = PMBSJob(cmd)
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
         host, tmp, n = slots[slotn]
         j.process = subprocess.Popen(["ssh","-x",host,j.cmd % n],bufsize=0,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
         fcntl.fcntl(j.process.stdout, fcntl.F_SETFL, os.O_NONBLOCK) 
         fcntl.fcntl(j.process.stderr, fcntl.F_SETFL, os.O_NONBLOCK) 
         j.id = jobid
         jobid += 1
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

