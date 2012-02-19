#!/usr/bin/env python
import os
from os.path import exists, join as pjoin
from glob import glob

boost_libs = "system filesystem program_options thread chrono"

def options(opt):
    opt.load('compiler_c compiler_cxx python boost unittest_gtest')
    opt.add_option('--with-snappy', default=None, help="Also looks for snappy at the given path")
    opt.add_option('--with-protobuf', default=None, help="Also looks for protobuf at the given path")
    opt.add_option('--with-cern-root-system', default=None, help="Also looks for the CERN Root System at the given path")

def configure(conf):
    conf.load('compiler_c compiler_cxx python boost unittest_gtest')
    conf.check_python_version((2,6,0))
    conf.parse_flags("-ldl -Wall -std=c++0x", uselib="CPP0X")
    conf.check(features='cxx cxxprogram', 
              lib=['m'],
              use=["CPP0X"],
              uselib_store='M')
    conf.env.append_value("CXXFLAGS", "-pthread")
    conf.env.append_value("LIB", "pthread")

    # find boost
    if not try_miniboost(conf):
        conf.check_boost(lib=boost_libs, mt=True)
    conf.start_msg("Using boost {0} libraries at".format(conf.env.BOOST_VERSION))
    conf.end_msg(",".join(conf.env.LIBPATH_BOOST+conf.env.STLIBPATH_BOOST), color="WHITE")

    # find protobuf
    if conf.options.with_protobuf:
        protobuf_pkg = pjoin(conf.options.with_protobuf, "lib/pkgconfig")
    else:
        protobuf_pkg = pjoin(os.getcwd(), "protobuf/lib/pkgconfig")
    pkgp = os.getenv("PKG_CONFIG_PATH")
    if os.getenv("PKG_CONFIG_PATH"):
        os.environ["PKG_CONFIG_PATH"] = ":".join((protobuf_pkg, os.getenv("PKG_CONFIG_PATH")))
    else:
        os.environ["PKG_CONFIG_PATH"] = protobuf_pkg
    conf.check_cfg(package="protobuf", atleast_version="2.4.1", uselib_store="PROTOBUF", args='--libs --cflags')

    # find snappy
    if conf.options.with_snappy:
        find_at(conf, "snappy", conf.options.with_snappy)
    elif not find_at(conf, "snappy", pjoin(os.getcwd(), "snappy")):
        conf.check_cxx(lib="snappy", uselib_store="snappy", mandatory=False)

    # find root
    root_config = "root-config"
    if conf.options.with_cern_root_system:
        root_config = pjoin(conf.options.with_cern_root_system, "bin/root-config")
    conf.check_cfg(path=root_config, package="", uselib_store="CERN_ROOT_SYSTEM", args='--libs --cflags')

    # We should test for these...
    conf.define("HAVE_CSTDINT", 1)
    #conf.define("HAVE_TR1_CSTDINT", 1)
    #conf.define("HAVE_STDINT_H", 1)
    conf.define("HAVE_CSTRING", 1)
    #conf.define("HAVE_TR1_CSTRING", 1)
    #conf.define("HAVE_STRING_H", 1)
    conf.define("HAVE_STD_SMART_PTR", 1)
    #conf.define("HAVE_STD_TR1_SMART_PTR", 1)

    for pack in ("a4io", "a4process", "a4hist", "a4atlas", "a4root"):
        conf.parse_flags("-I{0}/src".format(pack), uselib=pack.upper())
    conf.parse_flags("-L.", uselib="A4_LIBS")

    conf.to_log("Final environment:")
    conf.to_log(conf.env)
    conf.write_config_header('a4io/src/a4/config.h')

def add_pack(bld, pack, use=[]):
    proto_targets = []
    for pf in get_pfiles("%s/proto" % pack):
        used_packs = ["%s/proto" % p for p in [pack]+[u.lower() for u in use if u.startswith("A4")]]
        proto_targets.extend(add_proto(bld, pack, pf, used_packs))
    proto_cc = [f for f in proto_targets if f.endswith(".pb.cc")]
    lib_cppfiles = bld.path.ant_glob("%s/src/*.cpp" % pack)
    app_cppfiles = bld.path.ant_glob("%s/src/apps/*.cpp" % pack)
    test_cppfiles = bld.path.ant_glob("%s/src/tests/*.cpp" % pack)
    gtest_cppfiles = bld.path.ant_glob("%s/src/gtests/*.cpp" % pack)

    using = ["CPP0X", "PROTOBUF", "BOOST", pack.upper()] + use
    bld.stlib(source=proto_cc+lib_cppfiles, target=pack, vnum="0.1.0", use=using)
    bld.shlib(source=proto_cc+lib_cppfiles, target=pack, vnum="0.1.0", use=using)

    libs = [u.lower() for u in using if u.lower().startswith("a4")]
    using += ["A4_LIBS"]
    for app in app_cppfiles:
        bld.program(source=[app], target=str(app.change_ext("")), use=using, lib=libs)
    for app in test_cppfiles:
        bld.program(source=[app], target=str(app.change_ext("")), use=using, lib=libs)
    if gtest_cppfiles:
        bld.program(features="gtest", source=gtest_cppfiles, target="gtest_%s"%pack, use=using, lib=libs)

def build(bld):
    add_pack(bld, "a4io")
    add_pack(bld, "a4process", ["A4IO"])
    add_pack(bld, "a4hist", ["A4IO", "A4PROCESS", "CERN_ROOT_SYSTEM"])
    add_pack(bld, "a4root", ["A4IO", "A4PROCESS", "A4HIST", "CERN_ROOT_SYSTEM"])
    add_pack(bld, "a4atlas", ["A4ROOT", "A4HIST", "A4IO", "A4PROCESS"])

from os.path import basename, dirname
from os import walk

def add_proto(bld, pack, pf, includes):
    spack = pack.replace("a4", "a4/")
    pf_node = bld.path.find_resource(pf)
    pfd, pfn = dirname(pf), basename(pf)

    if pfd[:len(pjoin(pack, "proto", spack))] != pjoin(pack, "proto", spack):
        bld.fatal("Unexpected file: {0}".format(pfd))
    pfd = dirname(pf[len(pjoin(pack, "proto", spack))+1:])

    co = pjoin(pack, "src")
    po = pjoin(pack, "python")
    bld.path.find_or_declare(co)
    bld.path.find_or_declare(po)

    cpptargets = [pjoin(co, spack, pfd, pfn.replace(".proto", e)) for e in (".pb.cc", ".pb.h")]
    pytarget = [pjoin(po, spack, pfd, pfn.replace(".proto", e)) for e in ("_pb2.py",)]

    inc_paths = ["-I%s"%i for i in bld.env["INCLUDES_PROTOBUF"]]
    for i in includes:
        res = bld.path.find_node(i)
        rel_path = res.path_from(bld.path.get_bld())
        inc_paths.append("-I" + rel_path)
    incstr = " ".join(inc_paths)

    bld(rule="protoc %s --python_out %s --cpp_out %s ${SRC}" % (incstr, po, co),
        source=pf, target=" ".join(cpptargets+pytarget))

    return cpptargets + pytarget

def get_pfiles(d):
    pfiles = []
    for dname, dirs, files in walk(d):
        for f in files:
            if f.endswith(".proto"):
                pfiles.append(pjoin(dname, f))
    return pfiles

def find_at(conf, lib, where):
    if not exists(where):
        return False
    try:
        conf.env.stash()
        conf.env.append_value('RPATH', pjoin(where, "lib"))
        conf.parse_flags("-I{0}/include -L{0}/lib".format(where), uselib=lib.upper())
        conf.check_cxx(lib=lib, uselib_store=lib.upper(), use=[lib.upper()])
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

def try_miniboost(conf):
    miniboost_dir = pjoin(os.getcwd(),"miniboost")
    miniboost_lib = pjoin(miniboost_dir, "lib")
    if not exists(miniboost_dir) or not exists(miniboost_lib):
        conf.msg("Checking for builtin miniboost", "not found", color="YELLOW")
        return False
    try:
        conf.env.stash()
        conf.env.append_value('RPATH', miniboost_lib)
        conf.check_boost(lib=boost_libs, mt=True, includes=miniboost_dir, libs=miniboost_lib, abi="-a4")
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

