#! /usr/bin/env python

import os
import sys

oldpath = sys.path
sys.path.extend(os.environ["PATH"].split(":"))
import MergeNTUP_trf as ntups
sys.path = oldpath

ntups = [x[len("output"):-len("File")] for x in ntups.ListOfDefaultPositionalKeys
         if x.startswith("output") and "NTUP" in x]
         
if __name__ == "__main__":
    print "\n".join(ntups)

