#!/usr/bin/env python

boost_libs = "system filesystem program_options thread chrono"
a4_version = "0.1.0"

def options(opt):
    opt.load('compiler_c compiler_cxx python')
    opt.load('boost unittest_gtest libtool', tooldir="common/waf")
    opt.add_option('--with-protobuf', default=None,
        help="Also look for protobuf at the given path")
    opt.add_option('--with-cern-root-system', default=None,
        help="Also looks for the CERN Root System at the given path")
    opt.add_option('--with-snappy', default=None,
        help="Also look for snappy at the given path")

def configure(conf):
    import os
    from os.path import join as pjoin
    conf.load('compiler_c compiler_cxx python boost unittest_gtest libtool')
    conf.check_python_version((2,6,0))
    conf.find_program("doxygen", var="DOXYGEN", mandatory=False)

    # comment the following line for "production" run (not recommended)
    conf.env.append_value("CXXFLAGS", ["-g", "-Wall", "-Werror", "-ansi"])
    conf.env.append_value("CXXFLAGS", ["-std=c++0x"])
    conf.env.append_value("LDFLAGS", ["-Wl,--as-needed"])
    conf.env.append_value("RPATH", ["{0}/lib".format(conf.env.PREFIX)])
    conf.env.CXXFLAGS_OPTFAST = "-O2"
    conf.env.CXXFLAGS_OPTSIZE = "-Os"

    # find useful libraries
    conf.check(features='cxx cxxprogram', lib="m", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="dl", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="rt", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="pthread", uselib_store="DEFLIB")

    # find root
    root_cfg = "root-config"
    if conf.options.with_cern_root_system:
        root_cfg = pjoin(conf.options.with_cern_root_system, "bin/root-config")
    conf.check_cfg(path=root_cfg, package="", uselib_store="CERN_ROOT_SYSTEM",
        args='--libs --cflags')

    # find protobuf
    if conf.options.with_protobuf:
        protobuf_pkg = pjoin(conf.options.with_protobuf, "lib/pkgconfig")
        conf.env.PROTOC = pjoin(conf.options.with_protobuf, "bin/protoc")
    else:
        protobuf_pkg = pjoin(conf.path.abspath(), "protobuf/lib/pkgconfig")
        conf.env.PROTOC = pjoin(conf.path.abspath(), "protobuf/bin/protoc")
    pkgp = os.getenv("PKG_CONFIG_PATH")
    pkgp = pkgp + ":" if pkgp else ""
    os.environ["PKG_CONFIG_PATH"] = pkgp + protobuf_pkg
    conf.check_cfg(package="protobuf", atleast_version="2.4.0",
        uselib_store="PROTOBUF", args='--libs --cflags --variable=exec_prefix')

    # find snappy
    if conf.options.with_snappy:
        find_at(conf, "snappy", conf.options.with_snappy)
    elif not find_at(conf, "snappy", pjoin(conf.path.abspath(), "snappy")):
        conf.check_cxx(lib="snappy", uselib_store="snappy", mandatory=False)

    # find boost
    if not try_miniboost(conf):
        conf.check_boost(lib=boost_libs, mt=True)
    conf.start_msg("Using boost {0} libraries ".format(conf.env.BOOST_VERSION))
    boost_paths = conf.env.LIBPATH_BOOST + conf.env.STLIBPATH_BOOST
    conf.end_msg(",".join(boost_paths), color="WHITE")

    #conf.env.STATICLIB_A4STATIC = ['a4']
    #conf.env.LIBPATH_A4STATIC   = ['.'] 

    # We should test for these...
    conf.define("HAVE_CSTDINT", 1)
    #conf.define("HAVE_TR1_CSTDINT", 1)
    #conf.define("HAVE_STDINT_H", 1)
    conf.define("HAVE_CSTRING", 1)
    #conf.define("HAVE_TR1_CSTRING", 1)
    #conf.define("HAVE_STRING_H", 1)
    conf.define("HAVE_STD_SMART_PTR", 1)
    #conf.define("HAVE_STD_TR1_SMART_PTR", 1)
    conf.start_msg("Installation directory")
    conf.end_msg(conf.env.PREFIX, color="WHITE")

    conf.to_log("Final environment:")
    conf.to_log(conf.env)
    conf.write_config_header('a4io/src/a4/config.h')

def build(bld):
    if bld.cmd == 'doxygen':
        doc_packs(bld, ["a4io", "a4process", "a4hist", "a4atlas", "a4root"])
        return

    libsrc =  list(add_pack(bld, "a4io"))
    libsrc += add_pack(bld, "a4process", ["a4io"])
    libsrc += add_pack(bld, "a4hist",
        ["a4io", "a4process"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4root",
        ["a4io", "a4process", "a4hist", "a4atlas"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4atlas",
        ["a4io", "a4process", "a4hist", "a4root"])
    #bld(features="cxx cxxstlib", target="a4", name="a4static",
    #    vnum=a4_version, use=libsrc)
    #bld(features="cxx cxxshlib", target="a4", vnum=a4_version, use=libsrc)

def doc_packs(bld, packs):
    if not bld.env.DOXYGEN:
        bld.fatal("No doxygen executable found! Install doxygen and repeat ./waf configure.")
    srcs = []
    for p in packs:
        s = bld.path.find_or_declare("{0}/src".format(p))
        if not s:
            continue
        if s.get_src():
            srcs.append(s.get_src())
        if s.get_bld():
            srcs.append(s.get_bld())
    sourcedirs = " ".join(s.abspath() for s in srcs)
    udx = bld.path.find_or_declare("doc/user.doxygen.conf")
    ddx = bld.path.find_or_declare("doc/dev.doxygen.conf")
    bld(rule="cat ${SRC} | sed -e 's|__sourcedirs__|%s|' > ${TGT}" % sourcedirs,
        source="doc/user.doxygen", target=udx)
    bld(rule="cat ${SRC} | sed -e 's|__sourcedirs__|%s|' > ${TGT}" % sourcedirs,
        source="doc/dev.doxygen", target=ddx)
    bld(rule="${DOXYGEN} ${SRC}", source=udx, target="doc/user/html/index.html")
    bld(rule="${DOXYGEN} ${SRC}", source=ddx, target="doc/dev/html/index.html")

def add_pack(bld, pack, other_packs=[], use=[]):
    from os import listdir
    from os.path import join as pjoin

    # Add protoc rules
    proto_targets = []
    proto_includes = ["%s/proto" % p for p in [pack] + other_packs]
    for protof in bld.path.ant_glob("%s/proto/**/*.proto" % pack):
        proto_targets.extend(add_proto(bld, pack, protof, proto_includes))

    # Find all source files to be compiled
    proto_cc = [f for f in proto_targets if f.suffix() == ".cc"]
    proto_h = [f for f in proto_targets if f.suffix() == ".h"]
    lib_cppfiles = bld.path.ant_glob("%s/src/*.cpp" % pack)

    # Find applications and tests to be built
    apps = listdir("{0}/src/apps".format(pack))
    app_cppfiles = {}
    for app in apps:
        if app.endswith(".cpp"):
            app_cppfiles[app[:-4]] = ["{0}/src/apps/{1}".format(pack, app)]
        else:
            fls = bld.path.ant_glob("{0}/src/apps/{1}/*.cpp".format(pack, app))
            app_cppfiles[app] = fls
    test_cppfiles = bld.path.ant_glob("%s/src/tests/*.cpp" % pack)
    gtest_cppfiles = bld.path.ant_glob("%s/src/gtests/*.cpp" % pack)

    # Add compilation rules
    to_use = ["DEFLIB", "PROTOBUF", "BOOST"] + use
    incs = ["%s/src" % p for p in [pack] + other_packs]
    libnm = pjoin(pack, pack)

    # Build objects
    objs = []
    if lib_cppfiles:
        bld.objects(source=lib_cppfiles, target=libnm+"_obj", cxxflags="-fPIC",
            use=to_use+["OPTFAST"], includes=incs)
        objs.append(libnm+"_obj")
    if proto_cc:
        bld.objects(source=proto_cc, target=libnm+"_pbobj", cxxflags="-fPIC -Os",
            use=to_use, includes=incs)
        objs.append(libnm+"_pbobj")

    # Build libraries
    if objs:
        bld(features="cxx cxxstlib", target=libnm, vnum=a4_version,
            use=to_use + objs)
        bld(features="libtool cxx cxxshlib", target=libnm, vnum=a4_version,
            use=to_use + objs)

    # Build apps and tests
    opts = {}

    # link dynamically against shared sublibraries
    opts["use"] = to_use + [pjoin(p,p) for p in other_packs + [pack]]
    opts["use"] += [p.upper() for p in [pack] + other_packs]
    # link against liba4.so
    #opts["use"] = to_use + ["a4"]
    # link statically against liba4
    #opts["use"] = to_use + ["a4static"]

    testinst = "${PREFIX}/bin/a4tests"
    opts["includes"] = incs
    for app, fls in app_cppfiles.iteritems():
        if fls:
            bld.program(source=fls, target=pjoin(pack,app), **opts)
    for app in test_cppfiles:
        t = pjoin(pack, "test", str(app.change_ext("")))
        bld.program(source=[app], target=t, install_path=testinst, **opts)
    if gtest_cppfiles:
        t = pjoin(pack, "test", "gtest")
        bld.program(features="gtest", source=gtest_cppfiles, target=t,
            install_path=testinst, **opts)

    # install headers
    cwd = bld.path.find_node("{0}/src/a4".format(pack))
    if cwd:
        headers = cwd.ant_glob('**/*.h')
        if headers:
            bld.install_files('${PREFIX}/include/a4', cwd.ant_glob('**/*.h'),
                cwd=cwd, relative_trick=True)

    if proto_h:
        cwd = bld.path.find_or_declare("{0}/src/a4".format(pack)).get_bld()
        bld.install_files('${PREFIX}/include/a4', list(proto_h), cwd=cwd,
            relative_trick=True)

    return objs

def add_proto(bld, pack, pf_node, includes):
    from os.path import basename, dirname, join as pjoin
    spack = pack.replace("a4", "a4/")
    pfd, pfn = pf_node.path_from(bld.path.get_src()), str(pf_node)

    if pfd[:len(pjoin(pack, "proto", spack))] != pjoin(pack, "proto", spack):
        bld.fatal("Unexpected file: {0}".format(pfd))
    pfd = dirname(pfd[len(pjoin(pack, "proto", spack))+1:])

    co = pjoin(pack, "src")
    po = pjoin(pack, "python")
    bld.path.find_or_declare(co)
    bld.path.find_or_declare(po)

    cpptargets = [pjoin(co, spack, pfd, pfn.replace(".proto", e))
        for e in (".pb.cc", ".pb.h")]
    pytarget = [pjoin(po, spack, pfd, pfn.replace(".proto", e))
        for e in ("_pb2.py",)]

    inc_paths = ["-I%s"%i for i in bld.env["INCLUDES_PROTOBUF"]]
    for i in includes:
        res = bld.path.find_node(i)
        rel_path = res.path_from(bld.path.get_bld())
        inc_paths.append("-I" + rel_path)
    incs = " ".join(inc_paths)

    targets = [bld.path.find_or_declare(n) for n in cpptargets+pytarget]
    pc = bld.env.PROTOC if bld.env.PROTOC else "protoc"
    rule = "%s %s --python_out %s --cpp_out %s ${SRC}" % (pc, incs, po, co)
    bld(rule=rule, source=pf_node, target=targets)
    return targets

def find_at(conf, lib, where):
    from os.path import exists, join as pjoin
    if not exists(where):
        return False
    try:
        libn = lib.upper()
        conf.env.stash()
        conf.env.append_value('RPATH', pjoin(where, "lib"))
        conf.parse_flags("-I{0}/include -L{0}/lib".format(where), uselib=libn)
        conf.check_cxx(lib=lib, uselib_store=lib.upper(), use=[libn])
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

def try_miniboost(conf):
    from os.path import exists, join as pjoin
    miniboost_dir = pjoin(conf.path.abspath(),"miniboost")
    miniboost_lib = pjoin(miniboost_dir, "lib")
    miniboost_inc = pjoin(miniboost_dir, "include")
    if not exists(miniboost_dir) or not exists(miniboost_lib):
        conf.msg("Checking for builtin miniboost", "not found", color="YELLOW")
        return False
    try:
        conf.env.stash()
        conf.env.append_value('RPATH', miniboost_lib)
        conf.check_boost(lib=boost_libs, mt=True, includes=miniboost_inc,
            libs=miniboost_lib, abi="-a4")
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

from waflib import Build
class doxygen(Build.BuildContext):
    """generate doxygen documentation"""
    fun = 'build'
    cmd = 'doxygen'

