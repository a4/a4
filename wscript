#! /usr/bin/env python

from waflib.TaskGen import feature, after
from waflib.Task import Task
from waflib.Tools import c_preproc

def options(opt):
    opt.load('compiler_c compiler_cxx python')

def configure(conf):
    conf.load('compiler_c compiler_cxx python')
    
    conf.env.append_value("CXXFLAGS", ["-std=c++0x", "-ggdb"])
    conf.env.append_value("LDFLAGS", ["-Wl,--as-needed"])
    conf.env.append_value("RPATH", [conf.env.LIBDIR])
    
    conf.check_cfg(path="root-config", package="", uselib_store="CERN_ROOT_SYSTEM",
                   args='--libs --cflags', mandatory=False)

    conf.define("A4STORE_STANDALONE", 1)

    conf.to_log("Final environment:")
    conf.to_log(conf.env)

def build(bld):

    bld(features="cxx cxxshlib", 
        source=bld.path.ant_glob("src/**.cpp"),
        includes="src/",
        target="a4store")
    
    bld(features="cxx cxxprogram",
        source="src/apps/a4store_standalone_test.cpp",
        includes="src/",
        target="a4store_standalone_test",
        use=["a4store", "CERN_ROOT_SYSTEM"])
    
    headers = bld.path.ant_glob("src/a4/**.h")
    
    cwd = bld.path.find_or_declare("src/a4").get_src()
    bld.install_files('${PREFIX}/include/a4', headers, cwd=cwd,
        relative_trick=True)
