
class_ids = {}

def init():
    from os import listdir
    from os.path import abspath, isdir, dirname, join, exists
    fd = abspath(dirname(__file__))
    for dir in listdir(fd):
        if isdir(join(fd, dir)):
            dir = "a4.proto."+dir.strip()
            mod = __import__(dir, globals(), locals(), ["*"], -1)
            classes = (getattr(mod, k) for k in mod.__dict__.keys() if not k.startswith("_"))
            clsd = dict((c.CLASS_ID_FIELD_NUMBER, c) for c in classes if hasattr(c, "CLASS_ID_FIELD_NUMBER"))
            class_ids.update(clsd)

init()
del init
