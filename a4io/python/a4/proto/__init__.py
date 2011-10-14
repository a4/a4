
class_ids = {}

def init():
    from os import listdir, environ
    from os.path import abspath, isdir, dirname, join, exists
    import sys
    global class_ids
    fd = abspath(dirname(__file__))
    for dir in listdir(fd):
        if isdir(join(fd, dir)):
            dir = "a4.proto."+dir.strip()
            mod = __import__(dir, globals(), locals(), ["*"])
            classes = (getattr(mod, k) for k in mod.__dict__.keys() if not k.startswith("_"))
            clsd = dict((c.CLASS_ID_FIELD_NUMBER, c) for c in classes if hasattr(c, "CLASS_ID_FIELD_NUMBER"))
            class_ids.update(clsd)

    ppp = [abspath(p) for p in environ.get("A4_PROTOPYTHON_PATH","./python").split(":") if p and exists(p)]
    uppp = set()
    for p in ppp:
        if p in uppp:
            continue
        uppp.add(p)
        for dir in listdir(p):
            if isdir(join(p, dir)):
                sys.path.insert(0,p)
                dir = dir.strip()
                mod = __import__(dir, globals(), locals(), ["*"])
                classes = (getattr(mod, k) for k in mod.__dict__.keys() if not k.startswith("_"))
                clsd = dict((c.CLASS_ID_FIELD_NUMBER, c) for c in classes if hasattr(c, "CLASS_ID_FIELD_NUMBER"))
                class_ids.update(clsd)

    # Only allow <300
    class_ids = dict((a,b) for a,b in class_ids.iteritems() if a < 300)

init()
del init
