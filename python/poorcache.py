#!/usr/bin/env python
from os import getenv, listdir, stat, fork, waitpid, system
from os.path import abspath, isdir, join as pjoin
from sys import exit
from commands import getstatusoutput
from random import shuffle

my_scratch = "/scratch-local/ebke_cache"
grace = 10 # GB
    
def get_hosts():
    return [l.strip() for l in file(pjoin(getenv("HOME"), ".pmbs")).readlines() if l.strip()]

def scratch_info(host):
    s, o = getstatusoutput("ssh %s df -lP" % host)
    if s != 0:
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))
    for tup in o.split("\n")[1:]:
        fs, blk, used, avail, useperc, mp = tup.split()
        if not "dev" in fs or mp == "/":
            continue
        if mp == "/scratch-local":
            return int(avail)/1024.0/1024.0, int(blk)/1024.0/1024.0

def my_scratch_nuke(host):
    s, o = getstatusoutput("ssh %s rm -rf %s" % (host, my_scratch))
    if s != 0:
        if o.startswith("du: cannot access"):
            return 0.0
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))

def my_scratch_info(host):
    s, o = getstatusoutput("ssh %s du -s %s" % (host, my_scratch))
    if s != 0:
        if o.startswith("du: cannot access"):
            return 0.0
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))
    return int(o.split()[0])/1024.0/1024.0

def my_scratch_list(host):
    s, o = getstatusoutput("ssh %s find %s/ -not -type d " % (host, my_scratch))
    if s != 0:
        if "No such file" in o:
            return []
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))
    return [fn[len(my_scratch):] for fn in o.split("\n")]

def my_scratch_store(host, files):
    cmd = "ssh %s rsync --delete -vaR %s %s" % (host, ' '.join(files), my_scratch)
    s, o = getstatusoutput(cmd)
    if s != 0:
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))

def my_scratch_delete(host, files):
    cmd = "ssh %s rm %s" % (host, ' '.join(pjoin(my_scratch, f.lstrip("/")) for f in files))
    s, o = getstatusoutput(cmd)
    if s != 0:
        raise Exception("Could not SSH to %s, error %i: %s" % (host, s, o))

def list_files(dir):
    files = []
    dir = abspath(dir)
    if not isdir(dir):
        return [dir]
    for f in listdir(dir):
        fn = pjoin(dir, f)
        if isdir(fn):
            files.extend(list_files(fn))
        else:
            files.append(fn)
    return files
   
def get_file_sizes(files):
    return dict((fn, stat(fn).st_size/1024.0/1024.0/1024.0) for fn in files)

def cache(dirs):
    hosts = get_hosts()
    files = sum((list_files(dir) for dir in dirs), [])
    fsize = get_file_sizes(files)
    print "Caching %.3f GB in %i files..." % (sum(fsize.values()), len(files))

    file_set = set(files)
    file_distribution = {}
    free, total = {}, {}
    for host in hosts:
        free[host], total[host] = scratch_info(host)
        on_host = set(my_scratch_list(host))
        file_distribution[host] = on_host & file_set
        file_set.difference_update(on_host)
    print "%i files not yet cached. Distributing..." % len(file_set)
    
    for file in file_set:
        size = fsize[file]
        shuffle(hosts)
        full = True
        for host in hosts:
            if free[host] - grace > size:
                file_distribution[host].add(file)
                free[host] -= size
                full = False
                break
        if full:
            raise Exception("all hosts are full - please clean up!")
    print "Distribution: "
    for host in sorted(hosts):
        print host, " will have %i files with %.1f GB, then %.1f/%.1f GB free" % \
            (len(file_distribution[host]), sum(fsize[fn] for fn in file_distribution[host]), free[host], total[host])

    pids = []
    pidmap = {}
    for host in sorted(hosts):
        if len(file_distribution[host]) == 0:
            continue
        print "Storing on %s..." % host
        pid = fork()
        if pid == 0:
            my_scratch_store(host, sorted(file_distribution[host]))
            exit(0)
        pids.append(pid)
        pidmap[pid] = host
    while len(pids) > 0:
        pid, status = waitpid(-1, 0)
        pids.remove(pid)
        if status == 0:
            print "Host %s finished successfully!" % pidmap[pid]
        else:
            print "ERROR on host %s!" % pidmap[pid]
    print "Done."
    return file_distribution

def job_splitter(files, file_distribution):
    jobs = []
    file_set = set(files)
    for host, hfiles in file_distribution.iteritems():
        job = (host, [pjoin(my_scratch, f.lstrip("/")) for f in set(hfiles) & file_set])
        if job[1]:
            jobs.append(job)
        file_set.difference_update(hfiles)
    if len(file_set):
        raise Exception("Files not cached: %s" % file_set)
    return jobs

def scratch_overview():
    total_free, total_total, total_used_by_me = 0,0,0
    for host in get_hosts():
        free, total = scratch_info(host)
        used_by_me = my_scratch_info(host)
        print "%s: %.1f GB scratch, %.1f free, %.1f used by me" % (host, total, free, used_by_me)
        total_free += free
        total_total += total
        total_used_by_me += used_by_me
    print "-"*40
    print "TOTAL: %.1f GB scratch, %.1f free, %.1f used by me" % (total_total, total_free, total_used_by_me)
   

if __name__=="__main__":
    from sys import argv
    if len(argv) >= 2:
        cmd = argv[1]

    if cmd == "info":
        scratch_overview()
    if cmd == "cache":
        cache(argv[2:])
    if cmd == "kill":
        for host in get_hosts():
            system("ssh %s killall rsync" % host)
    if cmd == "status":
        for host in get_hosts():
            system("ssh %s ps aux | grep 12264" % host)

