import sys
from waflib.Configure import conf

@conf
def is_linux(conf):
    """
    `is_linux` returns true if we are building for linux

    FIXME: we should probably test a waf-based variant or configuration variable
    rather than sys.platform...
    """
    import sys
    return 'linux' in sys.platform.lower()

# EOF

