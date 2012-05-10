import os
from os.path import exists, join as pjoin
from waflib.Configure import conf

@conf
def find_at(conf, check, what, where, **kwargs):
    if not exists(where):
        return False
        
    pkgp = os.getenv("PKG_CONFIG_PATH", "")
    try:
        conf.env.stash()
        conf.env[what + "_HOME"] = where
        conf.env.append_value('PATH',  pjoin(where, "bin"))
        conf.env.append_value('RPATH', pjoin(where, "lib"))
        pkgconf_path = pjoin(where, "lib/pkgconfig")
        conf.env.append_value('PKG_CONFIG_PATH', pkgconf_path)
        conf.to_log("Pkg config path: %s" % conf.env.PKG_CONFIG_PATH)
        
        if pkgp: pkgp = pkgp + ":"
        os.environ["PKG_CONFIG_PATH"] = pkgconf_path + pkgp
        
        conf.parse_flags("-I{0}/include -L{0}/lib".format(where),
                         uselib=kwargs["uselib_store"])
        check(**kwargs)
        return True
    except conf.errors.ConfigurationError:
        os.environ["PKG_CONFIG_PATH"] = pkgp
        conf.end_msg("failed", color="YELLOW")
        conf.env.revert()
        return False

@conf
def check_with(conf, check, what, *args, **kwargs):
    """
    Perform `check`, also looking at --with-X commandline option and
    and X_HOME environment variable
    """
    import os
    
    with_dir = getattr(conf.options, "with_" + what, None)
    env_dir = os.environ.get(what.upper() + "_HOME", None)
    paths = [with_dir, env_dir] + kwargs.pop("extra_paths", [])
    
    what = what.upper()
    kwargs["uselib_store"] = kwargs.get("uselib_store", what)
    kwargs["use"] = kwargs.get("use", []) + [kwargs["uselib_store"]]
    
    for path in [p for p in paths if p]:
        conf.to_log("Checking in %s" % path)
        if conf.find_at(check, what, path, **kwargs):
            return
            
    check(**kwargs)
