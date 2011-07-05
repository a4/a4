#!/usr/bin/env python

from os import listdir, sep, makedirs, getcwd
from os.path import abspath, basename, dirname, isdir, join as pjoin

def get_dirs(d):
    jobs = []
    for f in listdir(d):
        if isdir(pjoin(d,f)):
            files.extend(get_files(pjoin(d,f)))
        else:
            files.append(pjoin(d,f))
    return files

def try_mkdir(fn):
    if fn:
        try:
            makedirs(dirname(fn))
        except OSError:
            pass


def get_a4_cmd(cmd, in_dir, res_out_d, skim_out_d):
    if not isdir(in_dir):
        raise Exception("Unexpected file: " + in_dir)
    name = in_dir.split('/')[-1]
    fls = [pjoin(in_dir,f) for f in listdir(in_dir) if f.endswith(".a4")]
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


def process(cmd, in_dirs, res_out_d=None, skim_out_d=None, dry_run=False):
    assert res_out_d or skim_out_d
    from os import getenv, getcwd
    opts = "cd %s; LD_LIBRARY_PATH=%s PATH=%s" % (getcwd(), getenv("LD_LIBRARY_PATH"), getenv("PATH"))
    cmds = [get_a4_cmd(" ".join((opts,cmd)), d, res_out_d, skim_out_d) for d in in_dirs]

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

if __name__=="__main__":
    from sys import argv
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-r", "--results", dest="results", default=None, help="result directory", metavar="DIR")
    parser.add_option("-s", "--skim", dest="skims", default=None, help="skim directory", metavar="DIR")
    parser.add_option("-n", "--nothing", dest="dryrun", default=False, help="do a dry run", metavar="DIR")
    (options, args) = parser.parse_args()
    cmd = args[0]
    inputs = args[1:]
    process(cmd, inputs, options.results, options.skims, options.dryrun)

