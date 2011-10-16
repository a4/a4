from io.A4_pb2 import fixed_class_id

fixed_class_ids = {}
def init():
    from os import listdir, environ
    from os.path import abspath, isdir, dirname, join, exists
    import sys
    global fixed_class_ids

    fd = abspath(dirname(__file__))
    for dir in listdir(fd):
        if isdir(join(fd, dir)):
            dir = "a4.proto."+dir.strip()
            mod = __import__(dir, globals(), locals(), ["*"])
            classes = (getattr(mod, k) for k in mod.__dict__.keys() if not k.startswith("_"))
            for c in classes:
                if isinstance(c, type(int)): # skip modules
                    opts = c.DESCRIPTOR.GetOptions()
                    if opts.HasExtension(fixed_class_id):
                        fixed_class_ids[opts.Extensions[fixed_class_id]] = c

init()
del init
del fixed_class_id
