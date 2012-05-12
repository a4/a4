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
        this_kwargs = kwargs.copy()
        this_kwargs['check_path'] = where
        check(**this_kwargs)
        return True
    except conf.errors.ConfigurationError:
        os.environ["PKG_CONFIG_PATH"] = pkgp
        conf.end_msg("failed", color="YELLOW")
        conf.env.revert()
        return False

@conf
def check_with(conf, check, what, *args, **kwargs):
    """
    Perform `check`, also looking at directories specified by the --with-X 
    commandline option and X_HOME environment variable (X = what.upper())
    
    The extra_args
    """
    import os
    from os.path import abspath
    
    with_dir = getattr(conf.options, "with_" + what, None)
    env_dir = os.environ.get(what.upper() + "_HOME", None)
    paths = [with_dir, env_dir] + kwargs.pop("extra_paths", [])
    
    WHAT = what.upper()
    kwargs["uselib_store"] = kwargs.get("uselib_store", WHAT)
    kwargs["use"] = kwargs.get("use", []) + [kwargs["uselib_store"]]
        
    for path in [abspath(p) for p in paths if p]:
        conf.to_log("Checking for %s in %s" % (what, path))
        if conf.find_at(check, WHAT, path, **kwargs):
            conf.msg("Found %s at" % what, path, color="WHITE")
            return
    
    check(**kwargs)
    conf.msg("Found %s at" % what, "(local environment)", color="WHITE")
