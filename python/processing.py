#!/usr/bin/env python

from os import listdir, sep, makedirs, getcwd, fork, waitpid, system
from os.path import abspath, basename, dirname, isdir, join as pjoin
from poorcache import cache, job_splitter, list_files

poor_cache = None

def try_mkdir(fn):
    if fn:
        try:
            makedirs(dirname(fn))
        except OSError:
            pass

def get_a4_cmd(cmd, fls, name, res_out_d, skim_out_d):
    out_skim = pjoin(skim_out_d, name + ".a4") if skim_out_d else None
    out_res  = pjoin(res_out_d , name + ".results") if res_out_d else None
    try_mkdir(out_skim)
    try_mkdir(out_res)
    cmdl = [cmd]
    cmdl.append("--threads=%i") # will be filled in PMBS
    if out_skim:
        cmdl.append("-o %s" % out_skim)
    if out_res:
        cmdl.append("-r %s" % out_res)
    cmdl.extend(fls)
    return " ".join(cmdl)

def get_a4_cmds(cmd, in_dirs, res_out_d=None, skim_out_d=None, use_poorcache=True):
    assert res_out_d or skim_out_d
    from os import getenv, getcwd
    opts = "cd %s; LD_LIBRARY_PATH=%s PATH=%s" % (getcwd(), getenv("LD_LIBRARY_PATH"), getenv("PATH"))
    cmds = []
    for d in in_dirs:
        name = d.split('/')[-1]
        fls = [f for f in list_files(d) if f.endswith(".a4")]
        #fls = [abspath(pjoin(d,f)) for f in listdir(d) if f.endswith(".a4")]
        if use_poorcache:
            for host, files in job_splitter(fls, poor_cache):
                cmds.append((host, get_a4_cmd(" ".join((opts,cmd)), files, "_".join((name,host)), res_out_d, skim_out_d)))
        else:
            cmds.append(get_a4_cmd(" ".join((opts,cmd)), fls, name, res_out_d, skim_out_d, use_poorcache))
    return cmds

def process(cmds, dry_run=False):
    if dry_run:
        for cmd in cmds[::-1]:
            print cmd
        return
    import pmbs
    pmbs.filter.append("SmearingClass INFO:: Using default path!")
    
    for cmd in cmds[::-1]:
       pmbs.submit(cmd)
    pmbs.flush()
    print "Processing completed."


n_threads = 2
if __name__=="__main__":
    from sys import argv
    from optparse import OptionParser
    from glob import glob
    parser = OptionParser()
    parser.add_option("-r", "--results", dest="results", default=None, help="result directory", metavar="DIR")
    parser.add_option("-o", "--skim", dest="skims", default=None, help="skim directory", metavar="DIR")
    parser.add_option("-n", "--nothing", dest="dryrun", action="store_true", default=False, help="do a dry run")
    parser.add_option("-p", "--poorcache", dest="poorcache", action="store_true", default=True, help="use the poor cache")
    parser.add_option("-d", "--defaultdir", dest="defaultdir", default=None, help="specify a dir with /mc /egamma and /muons subdirectories, which will be processed by cmd_mc, cmd_egamma and cmd_muons", metavar="DIR")
    (options, args) = parser.parse_args()

    if options.dryrun:
        print "DRY RUN..."

    cmd = args[0]
    inputs = args[1:]
    print options.results
    if options.defaultdir:
        assert len(inputs) == 0
        input = options.defaultdir
        if options.poorcache:
            poor_cache = cache([input])
        cmds = []
        assert isdir(pjoin(input, "mc"))
        assert isdir(pjoin(input, "egamma"))
        assert isdir(pjoin(input, "muons"))
        res_mc = pjoin(options.results, "mc") if options.results else None
        skm_mc = pjoin(options.skims, "mc") if options.skims else None
        res_egamma = pjoin(options.results, "egamma") if options.results else None
        skm_egamma = pjoin(options.skims, "egamma") if options.skims else None
        res_muons = pjoin(options.results, "muons") if options.results else None
        skm_muons = pjoin(options.skims, "muons") if options.skims else None
        cmds.extend(get_a4_cmds(cmd + "_mc", glob(pjoin(input, "mc","*")), res_mc, skm_mc, options.poorcache))
        cmds.extend(get_a4_cmds(cmd + "_egamma", glob(pjoin(input, "egamma","*")), res_egamma, skm_egamma, options.poorcache))
        cmds.extend(get_a4_cmds(cmd + "_muons", glob(pjoin(input, "muons","*")), res_muons, skm_muons, options.poorcache))
    else:
        if options.poorcache:
            poor_cache = cache(inputs)
        cmds = get_a4_cmds(cmd, inputs, options.results, options.skims, options.poorcache)

    if not options.poorcache:
        process(cmds, options.dryrun)
    else:
        pids = []
        pidmap = {}
        hosts = sorted(set(h for h, cmd in cmds))

        for host in hosts:
            host_commands = [cmd%n_threads for h, cmd in cmds if h == host]
            if options.dryrun:
                print "running %i commands on %s.." % (len(host_commands), host)
                continue

            print "Starting to run on %s..." % host
            pid = fork()
            if pid == 0:
                for cmd in host_commands:
                    system("ssh %s '%s'" % (host, cmd))
                exit(0)
            pids.append(pid)
            pidmap[pid] = host

        if not options.dryrun:
            while len(pids) > 0:
                pid, status = waitpid(-1, 0)
                pids.remove(pid)
                if status == 0:
                    print "Host %s finished successfully - %i/%i remaining." % (pidmap[pid], len(pids), len(pidmap))
                else:
                    print "ERROR on host %s!" % pidmap[pid]
        print "Done."

