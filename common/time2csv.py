#! /usr/bin/env python

from commands import getstatusoutput
from csv import writer
from datetime import datetime
from socket import gethostname
from os import walk
from os.path import basename, join as pjoin

def gen(f, infd):
    content = [l.strip() for l in infd.read().split("\n") if l.startswith("\t")]
    line, content = content[0], content[1:]
    
    content = [(a, b) for a, _, b in (l.rpartition(": ") for l in content)]
    headings, content = zip(*content)
    
    headings = ("Path", "File",) + headings
    content = (f, basename(f)) + content
    
    return headings, content


def main(args):
    rows = []
    for i, f in enumerate(args):
        with open(f) as fd:
            headings, content = gen(f, fd)
            rows.append(content)
        
    #sort_idx = headings.index("User time (seconds)")
    sort_idx = headings.index("Maximum resident set size (kbytes)")
    rows.sort(reverse=True, key=lambda x: float(x[sort_idx]))
            
    status, githash = getstatusoutput("git rev-parse HEAD")
    if status:
        githash = "unk"
        print "Git rev-parse HEAD failed: ", status
    
    host = gethostname().partition(".")[0]
    date = datetime.now().strftime("%Y-%m-%d")
    output_filename = "{0}-{1}-{2}.csv".format(date, host, githash[:6])
    output_filename = "common/compiletimes/" + output_filename
                
    with open(output_filename, "w") as outfd:
        output = writer(outfd)
        output.writerows([headings] + rows)
    print "Wrote", output_filename

def find_time_files():
    all_files = []
    for dirname, dirs, files in walk("."):
        all_files.extend(pjoin(dirname, f) for f in files if f.endswith(".time"))
    return all_files

if __name__ == "__main__":
    from sys import argv
    if len(argv) > 1:
        main(argv[1:])
    else:
        main(find_time_files())
