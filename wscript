#!/usr/bin/env python

boost_libs = "system filesystem program_options thread chrono"
a4_version = "0.1.0"

def go(ctx):
    from waflib.Options import commands, options
    from os import getcwd
    from os.path import join as pjoin
    options.prefix = pjoin(getcwd(), "install")
    commands += ["configure", "build", "install"]

def onchange(ctx):
    """
    Detect changes to source files then run the following waf, e.g. "./waf onchange build"
    """
    
    from sys import argv
    argv = argv[1:]
    assert argv[0] == "onchange"
    argv = argv[1:]

    from pipes import quote
    args = " ".join(quote(arg) for arg in argv)

    from waflib.Options import commands
    commands[:] = []
    
    # System because
    from os import system
    system("common/autocompile.py ./waf {0}".format(args))
        
    raise SystemExit()

def options(opt):
    opt.load('compiler_c compiler_cxx python')
    opt.load('boost unittest_gtest libtool', tooldir="common/waf")
    opt.add_option('--with-protobuf', default=None,
        help="Also look for protobuf at the given path")
    opt.add_option('--with-cern-root-system', default=None,
        help="Also looks for the CERN Root System at the given path")
    opt.add_option('--with-snappy', default=None,
        help="Also look for snappy at the given path")
    opt.add_option('--with-boost', default=None,
        help="Also look for boost at the given path")

def configure(conf):
    import os
    from os.path import join as pjoin
    conf.load('compiler_c compiler_cxx python')
    conf.load('boost unittest_gtest libtool', tooldir="common/waf")

    conf.cc_add_flags()
    conf.check_python_version((2,6,0))
    conf.find_program("doxygen", var="DOXYGEN", mandatory=False)

    # comment the following line for "production" run (not recommended)
    conf.env.append_value("CXXFLAGS", ["-g", "-Wall", "-Werror", "-ansi", "-fno-strict-aliasing"])
    conf.env.append_value("CXXFLAGS", ["-std=c++0x"])
    conf.env.append_value("LDFLAGS", ["-Wl,--as-needed"])
    conf.env.append_value("RPATH", [conf.env.LIBDIR])
    conf.env.CXXFLAGS_OPTFAST = "-O2"
    conf.env.CXXFLAGS_OPTSIZE = "-Os"
    conf.env.A4_VERSION = a4_version

    # find useful libraries
    conf.check(features='cxx cxxprogram', lib="m", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="dl", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="rt", uselib_store="DEFLIB")
    conf.check(features='cxx cxxprogram', lib="pthread", uselib_store="DEFLIB")
    
    check_have_atomic(conf)
    
    conf.check_cxx(
        msg="Checking for C++11 lambda syntax",
        fragment="""int main(int argc, char* argv[]) {
                        volatile int a = 0;
                        auto x = [&]() { return a; };
                        return x();
                    }""",
        define_name="HAVE_LAMBDA",
        mandatory=False)
        
    conf.check_cxx(
        msg="Checking for C++11 noexcept keyword",
        fragment="""int blarg() noexcept { return 2; }
                    int main(int argc, char* argv[]) {
                        return blarg();
                    }""",
        define_name="HAVE_NOEXCEPT",
        mandatory=False)
        
    conf.check_cxx(
        msg="Checking for C++11 initializer lists",
        fragment="""
            #include <vector>
            int myfunc(const std::initializer_list<int>& x) {
                std::vector<int> v(x);
                return v.front();
            }
        
            int main(int argc, char* argv[]) {
                return myfunc({0, 1, 2});
            }""",
        define_name="HAVE_INITIALIZER_LISTS",
        mandatory=False)

    # find root
    root_cfg = "root-config"
    if conf.options.with_cern_root_system:
        root_cfg = pjoin(conf.options.with_cern_root_system, "bin/root-config")
    conf.check_cfg(path=root_cfg, package="", uselib_store="CERN_ROOT_SYSTEM",
        args='--libs --cflags', mandatory=False)

    # find protobuf
    pb_bin = []
    if conf.options.with_protobuf:
        protobuf_pkg = pjoin(conf.options.with_protobuf, "lib/pkgconfig")
        pb_bin.append(pjoin(conf.options.with_protobuf, "bin"))
    else:
        protobuf_pkg = pjoin(conf.path.abspath(), "protobuf/lib/pkgconfig")
        pb_bin.append(pjoin(conf.path.abspath(), "protobuf/bin"))
    pkgp = os.getenv("PKG_CONFIG_PATH", "")
    if pkgp:
        pkgp = pkgp + ":"
    os.environ["PKG_CONFIG_PATH"] = pkgp + protobuf_pkg
    conf.check_cfg(package="protobuf", atleast_version="2.4.0",
        uselib_store="PROTOBUF", args='--libs --cflags')
    conf.find_program("protoc", var="PROTOC", path_list=pb_bin)
    if conf.env.LIBPATH_PROTOBUF:
        conf.env.append_value('RPATH', conf.env.LIBPATH_PROTOBUF[0])

    # find snappy
    if conf.options.with_snappy:
        find_at(conf, "snappy", conf.options.with_snappy)
    elif not find_at(conf, "snappy", pjoin(conf.path.abspath(), "snappy")):
        conf.check_cxx(lib="snappy", uselib_store="snappy", mandatory=False)

    # find boost
    if conf.options.with_boost:
        if not try_boost_path(conf, conf.options.with_boost):
            conf.fatal("Could not find boost at %s" % conf.options.with_boost)
    else:
        if not try_boost_path(conf):
            conf.check_boost(lib=boost_libs, mt=True)

    # print locations of used libraries
    if conf.env.LIBPATH_SNAPPY:
        loc = conf.env.LIBPATH_SNAPPY[0]
        conf.msg("Using snappy library ", loc, color="WHITE")
        conf.define("HAVE_SNAPPY", 1)
        
    loc = conf.env.LIBPATH_PROTOBUF
    if loc:
        loc = loc[0]
    else:
        loc = "(installed as system library)"
    conf.msg("Using protobuf library ", loc, color="WHITE")
    
    boost_paths = conf.env.LIBPATH_BOOST + conf.env.STLIBPATH_BOOST
    conf.msg("Using boost {0} libraries ".format(conf.env.BOOST_VERSION),
        ",".join(boost_paths), color="WHITE")

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

def check_have_atomic(conf):
    conf.check_cxx(
        msg="Checking for std::atomic",
        fragment="""
            #include <atomic>
            int main(int argc, char* argv[]) {
                std::atomic<int> a;
                volatile int x = 1;
                a += x;
                return a;
            }
        """,
        define_name="HAVE_ATOMIC",
        mandatory=False)

def build(bld):
    from os.path import join as pjoin
    packs = ["a4io", "a4store", "a4process", "a4hist", "a4atlas", "a4root", "a4plot"]

    if bld.cmd == 'doxygen':
        doc_packs(bld, packs)
        return

    libsrc =  list(add_pack(bld, "a4io", [], ["SNAPPY"]))
    libsrc += add_pack(bld, "a4store", ["a4io"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4process", ["a4io", "a4store"])
    libsrc += add_pack(bld, "a4hist",
        ["a4io", "a4store", "a4process"], ["CERN_ROOT_SYSTEM"])
    if "HAVE_CERN_ROOT_SYSTEM=1" in bld.env.DEFINES:
        libsrc += add_pack(bld, "a4root",
            ["a4io", "a4store", "a4process", "a4hist", "a4atlas"], ["CERN_ROOT_SYSTEM"])
    libsrc += add_pack(bld, "a4atlas",
        ["a4io", "a4store", "a4process", "a4hist", "a4root"])
    #bld(features="cxx cxxstlib", target="a4", name="a4static",
    #    vnum=a4_version, use=libsrc)
    #bld(features="cxx cxxshlib", target="a4", vnum=a4_version, use=libsrc)
    
    # Install configuration header
    ch = bld.path.find_resource("a4io/src/a4/config.h")
    bld.install_files("${PREFIX}/include/a4", ch)

    # Install binaries
    bld.install_files("${BINDIR}", bld.path.ant_glob("a4*/bin/*"), chmod=0755)

    # Install python modules
    for pack in packs:
        cwd = bld.path.find_node("{0}/python".format(pack))
        if cwd:
            files = cwd.ant_glob('**/*.py')
            if files:
                bld.install_files('${PYTHONDIR}', files, cwd=cwd, relative_trick=True)

    # Create this_a4.sh and packageconfig files, symlink python
    bld(rule=write_this_a4, target="this_a4.sh", install_path=bld.env.BINDIR)
    bld(rule=write_pkgcfg, target="a4.pc", install_path=pjoin(bld.env.LIBDIR, "pkgconfig"))
    #bld.symlink_as(pjoin(bld.env.BINDIR, "python"), bld.env.PYTHON[0])

    # Add post-install checks
    if bld.is_install:
        do_installcheck(bld)


def write_pkgcfg(task):
    def libstr(use):
        s = []
        if task.env["LIBPATH_"+use]:
            s.extend("-L%s"%l for l in task.env["LIBPATH_"+use])
            s.extend("-l%s"%l for l in task.env["LIB_"+use])
        return " ".join(s)

    def cppstr(use):
        s = []
        s.extend(task.env.get_flat("CPPFLAGS_"+use).split())
        s.extend("-I"+i for i in task.env.get_flat("INCLUDES_"+use).split())
        return " ".join(s)

    lines = []
    from textwrap import dedent
    lines.append(dedent("""
    prefix={PREFIX}
    exec_prefix=${{prefix}}
    includedir=${{prefix}}/include
    libdir={LIBDIR}
    CXX={CXX}
    PROTOC={PROTOC}

    Name: A4
    Description: An Analysis Tool for High-Energy Physics
    URL: https://github.com/JohannesEbke/a4
    Version: {A4_VERSION}
    Cflags: -std=c++0x -I{PREFIX}/include {CPPFLAGS_PROTOBUF} {CPPFLAGS_BOOST} {CPPFLAGS_SNAPPY}
    Libs: -L${{libdir}} -la4root -la4hist -la4process -la4store -la4io {protobuflibs} {boostlibs} {snappylibs}
    Requires: protobuf >= 2.4
    """.format(
        PREFIX=task.env.PREFIX, 
        LIBDIR=task.env.LIBDIR, 
        CXX=task.env.CXX[0], 
        PROTOC=task.env.PROTOC, 
        A4_VERSION=task.env.A4_VERSION, 
        CPPFLAGS_PROTOBUF=cppstr("PROTOBUF"),
        CPPFLAGS_BOOST=cppstr("BOOST"),
        CPPFLAGS_SNAPPY=cppstr("SNAPPY"),
        protobuflibs=libstr("PROTOBUF"),
        boostlibs=libstr("BOOST"),
        snappylibs=libstr("SNAPPY")
    )))

    task.outputs[0].write("\n".join(lines))
    return 0

def write_header_test(header, cwd):
    include = header.path_from(cwd).lstrip("./")
    def writer(task):
        lines = []
        assert len(task.inputs) == 1
        header = task.inputs[0]
        lines.append("// Check if header is self-sufficient")
        lines.append("#include <a4/{0}>".format(include))
        lines.append("int main() { return 0; }")
        task.outputs[0].write("\n".join(lines))
    return writer

def add_header_test(bld, cwd, header, opts):
    test_cxx = header.change_ext('_standalone_test.cpp')
    test_exe = header.change_ext('_standalone_test')
    bld(rule=write_header_test(header, cwd), source=[header], target=test_cxx)
    bld.program(features="testt", source=[test_cxx], target=test_exe, **opts)

def write_this_a4(task):
    import os
    lines = []
    if task.env.LIBPATH_PROTOBUF:
        lines.append("# Setup protobuf since it is not installed")
        lines.append("export PKG_CONFIG_PATH=${{PKG_CONFIG_PATH:+$PKG_CONFIG_PATH:}}{0}/pkgconfig"
                     .format(task.env.LIBPATH_PROTOBUF[0]))
        pb_root = os.sep.join(task.env.LIBPATH_PROTOBUF[0].split(os.sep)[:-1])
    lines.append("export PYTHONPATH={0}${{PYTHONPATH:+:$PYTHONPATH}}"
                 .format(os.path.join(pb_root, "python")))

    from textwrap import dedent
    lines.append(dedent("""
    export PKG_CONFIG_PATH=${{PKG_CONFIG_PATH:+$PKG_CONFIG_PATH:}}{LIBDIR}/pkgconfig
    export PYTHONPATH={PYTHONDIR}${{PYTHONPATH:+:$PYTHONPATH}}
    export PATH={BINDIR}${{PATH:+:$PATH}}

    # Get the used compiler with $(pkg-config a4 --variable=CXX)
    # Get the used protoc compiler with $(pkg-config a4 --variable=PROTOC)

    # In Makefiles, do not forget that the syntax is:
    # CXX=$(shell pkg-config a4 --variable=CXX)
    #""".format(
        PYTHONDIR=task.env.PYTHONDIR,
        LIBDIR=task.env.LIBDIR,
        BINDIR=task.env.BINDIR)))

    task.outputs[0].write("\n".join(lines))
    return 0

def doc_packs(bld, packs):
    if not bld.env.DOXYGEN:
        bld.fatal("No doxygen executable found! Install doxygen and repeat ./waf configure.")
    srcs = [bld.path.find_node("doc/doxygen")]
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
    from os.path import dirname, join as pjoin

    # Add protoc rules
    proto_sources = bld.path.ant_glob("%s/proto/**/*.proto" % pack)
    if pack == "a4atlas":
        proto_sources = [s for s in proto_sources
                         if not ("_flat" in s.srcpath() and
                                  "ntup" in s.srcpath())]
    proto_targets = []
    proto_includes = ["%s/proto" % p for p in [pack] + other_packs]
    for protof in proto_sources:
        proto_targets.extend(add_proto(bld, pack, protof, proto_includes))

    # Find all source files to be compiled
    proto_cc = [f for f in proto_targets if f.suffix() == ".cc"]
    proto_h = [f for f in proto_targets if f.suffix() == ".h"]
    proto_py = [f for f in proto_targets if f.suffix() == ".py"]
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
    test_scripts = bld.path.ant_glob("%s/src/tests/*.sh" % pack)
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
            install_path="${LIBDIR}", use=to_use + objs)
        bld(features="libtool cxx cxxshlib", target=libnm, vnum=a4_version,
            install_path="${LIBDIR}", use=to_use + objs)

    # Set app and test options
    opts = {}
    # link dynamically against shared sublibraries
    opts["use"] = to_use + [pjoin(p,p) for p in other_packs + [pack]]
    opts["use"] += [p.upper() for p in [pack] + other_packs]
    # link against liba4.so
    #opts["use"] = to_use + ["a4"]
    # link statically against liba4
    #opts["use"] = to_use + ["a4static"]

    # Build applications
    opts["includes"] = incs
    for app, fls in app_cppfiles.iteritems():
        if fls:
            bld.program(source=fls, target=pjoin(pack,app), **opts)

    opts["install_path"] = None
    # Build test applications
    testapps = []
    for app in test_cppfiles:
        t = pjoin(pack, "tests", str(app.change_ext("")))
        t = bld.path.find_or_declare(t)
        if str(app).startswith("test_"):
            is_test = "testt"
        else:
            is_test = ""
        bld.program(features=is_test, source=[app], target=t, **opts)
        testapps.append(t)

    # Run test scripts
    for testscript in test_scripts:
        if str(testscript).startswith("test_"):
            t = pjoin(pack, "tests", str(testscript))
            t = bld.path.find_or_declare(t)
            tsk = bld(features="testsc", rule="cp ${SRC} ${TGT} && chmod +x ${TGT}",
                source=testscript, target=t,
                use=testapps)

    # Build and run gtests
    if gtest_cppfiles:
        t = pjoin(pack, "tests", "gtest")
        bld.program(features="gtest", source=gtest_cppfiles, target=t, **opts)

    # install headers
    cwd = bld.path.find_node("{0}/src/a4".format(pack))
    if cwd:
        headers = cwd.ant_glob('**/*.h')
        if headers:
            bld.install_files('${PREFIX}/include/a4', cwd.ant_glob('**/*.h'),
                cwd=cwd, relative_trick=True)
            for h in headers:
                add_header_test(bld, cwd, h, opts)


    if proto_sources:
        cwd = bld.path.find_or_declare("{0}/proto/a4".format(pack)).get_src()
        bld.install_files('${PREFIX}/include/a4', proto_sources, cwd=cwd,
            relative_trick=True)
    
    if proto_h:
        cwd = bld.path.find_or_declare("{0}/src/a4".format(pack)).get_bld()
        bld.install_files('${PREFIX}/include/a4', proto_h, cwd=cwd,
            relative_trick=True)

    if proto_py:
        cwd = bld.path.find_or_declare("{0}/python".format(pack)).get_bld()
        paths = set(dirname(n.path_from(bld.path.get_bld())) for n in proto_py)
        initfiles = [bld.path.find_or_declare(pjoin(p,"__init__.py")) for p in paths]
        bld(rule="touch ${TGT}", target=initfiles)
        bld.install_files('${PYTHONDIR}', proto_py+initfiles, cwd=cwd,
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
        if res:
            rel_path = res.path_from(bld.path.get_bld())
            inc_paths.append("-I" + rel_path)
    incs = " ".join(inc_paths)

    targets = [bld.path.find_or_declare(n) for n in cpptargets+pytarget]
    if bld.env.PROTOC:
        pc = bld.env.PROTOC
    else:
        pc = "protoc"
    rule = "%s %s --python_out %s --cpp_out %s ${SRC}" % (pc, incs, po, co)
    bld(rule=rule, source=pf_node, target=targets)
    return targets

def find_at(conf, lib, where, static=False):
    from os.path import exists, join as pjoin
    if not exists(where):
        return False
    try:
        libn = lib.upper()
        conf.env.stash()
        if not static:
            conf.env.append_value('RPATH', pjoin(where, "lib"))
        conf.parse_flags("-I{0}/include -L{0}/lib".format(where), uselib=libn,
            force_static=static)
        conf.check_cxx(lib=lib, uselib_store=lib.upper(), use=[libn])
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

def try_boost_path(conf, boost_path=None):
    from os.path import exists, join as pjoin
    if boost_path is None:
        boost_path = pjoin(conf.path.abspath(),"miniboost")
    boost_lib = pjoin(boost_path, "lib")
    boost_inc = pjoin(boost_path, "include")
    conf.msg("Checking for boost at", boost_path, color="WHITE")
    if not exists(boost_path) or not exists(boost_lib):
        conf.msg("Checking for boost at %s"%boost_path, "not found", color="YELLOW")
        return False
    try:
        conf.env.stash()
        conf.env.append_value('RPATH', boost_lib)
        conf.check_boost(lib=boost_libs, mt=True, includes=boost_inc,
            libs=boost_lib, abi="-a4")
        return True
    except conf.errors.ConfigurationError:
        conf.end_msg("failed",color="YELLOW")
        conf.env.revert()
        return False

def do_installcheck(bld):
    import os
    # set test env
    keys = bld.env.get_merged_dict().keys()

    import os
    os.environ["BINDIR"] = bld.env.BINDIR
    os.environ["SRCDIR"] = bld.path.get_src().abspath()

    test_scripts = bld.path.ant_glob("*/src/tests/install_*.sh")
    for script in test_scripts:
        pack = script.path_from(bld.path).split(os.sep)[0]
        t = os.path.join(pack, "tests", str(script))
        t = bld.path.find_or_declare(t)
        bld(features="testsc", rule="cp ${SRC} ${TGT} && chmod +x ${TGT}",
            source=script.get_src(), target=t, env=bld.env)


from waflib import Build
class doxygen(Build.BuildContext):
    """generate doxygen documentation"""
    fun = 'build'
    cmd = 'doxygen'

