import zlib

class ZlibOutputStream(object):
    def __init__(self, out_stream):
        self.dco = zlib.compressobj(9)
        self.out_stream = out_stream
        self.bytes_written = 0

    def write(self, data):
        compressed = self.dco.compress(data)
        self.bytes_written += len(compressed)
        self.out_stream.write(compressed)

    def close(self):
        compressed = self.dco.flush()
        self.bytes_written += len(compressed)
        self.out_stream.write(compressed)

class ZlibInputStream(object):
    def __init__(self, in_stream, reqsize=64*1024):
        self.dco = zlib.decompressobj()
        self.in_stream = in_stream
        self.reqsize = reqsize
        self.decbuf = ""
        tell = self.in_stream.tell()
        self.tell = lambda : tell

    def read(self, bytes):
        while len(self.decbuf) < bytes:
            assert self.dco.unused_data == ""
            avail_bytes = self.in_stream.read(self.reqsize)
            self.decbuf += self.dco.decompress(avail_bytes)
        rv = self.decbuf[:bytes]
        self.decbuf = self.decbuf[bytes:]
        return rv

    def close(self):
        self.dco.flush()
        if self.dco.unused_data:
            self.in_stream.seek(-len(self.dco.unused_data), 1)
