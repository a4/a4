from struct import pack, unpack
from bisect import bisect_right as bisect
import zlib

from a4.proto import class_ids
from a4.proto.io import A4StreamHeader, A4StreamFooter, A4StartCompressedSection, A4EndCompressedSection

START_MAGIC = "A4STREAM"
END_MAGIC = "KTHXBYE4"
HIGH_BIT = (1<<31)
SEEK_END = 2

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

class A4OutputStream(object):
    def __init__(self, out_stream, description=None, content_cls=None, metadata_cls=None, compression=True, metadata_refers_forward=False):
        self.compression = True
        self.compressed = False
        self.bytes_written = 0
        self.content_count = 0
        self.content_class_id = None
        self._raw_out_stream = out_stream
        self.out_stream = out_stream
        self.content_class_id = None
        self.metadata_class_id = None
        self.metadata_refers_forward = metadata_refers_forward
        if content_cls:
            self.content_class_id = content_cls.CLASS_ID_FIELD_NUMBER
        if metadata_cls:
            self.metadata_class_id = metadata_cls.CLASS_ID_FIELD_NUMBER
        self.metadata_offsets = []
        self.write_header(description)
        self.start_compression()

    def start_compression(self):
        assert not self.compressed
        sc = A4StartCompressedSection()
        sc.compression = sc.ZLIB
        self.write(sc)
        self._pre_compress_bytes_written = self.bytes_written
        self.out_stream = ZlibOutputStream(self._raw_out_stream)
        self.compressed = True
    
    def stop_compression(self):
        assert self.compressed
        sc = A4EndCompressedSection()
        self.write(sc)
        self.out_stream.close()
        self.bytes_written = self.out_stream.bytes_written + self._pre_compress_bytes_written
        self.out_stream = self._raw_out_stream
        self.compressed = False

    def write_header(self, description=None):
        self.out_stream.write(START_MAGIC)
        self.bytes_written += len(START_MAGIC)
        header = A4StreamHeader()
        header.a4_version = 1
        header.metadata_refers_forward = self.metadata_refers_forward
        if description:
            header.description = description
        if self.content_class_id:
            header.content_class_id = self.content_class_id
        if self.metadata_class_id:
            header.metadata_class_id = self.metadata_class_id
        self.write(header)

    def write_footer(self):
        footer = A4StreamFooter()
        footer.size = self.bytes_written
        footer.metadata_offsets.extend(self.metadata_offsets)
        footer.metadata_refers_forward = self.metadata_refers_forward
        if self.content_class_id:
            footer.content_count = self.content_count
        self.write(footer)
        self.out_stream.write(pack("<I", footer.ByteSize()))
        self.out_stream.write(END_MAGIC)
        self.bytes_written += len(END_MAGIC)

    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    def close(self):
        if self.compressed:
            self.stop_compression()
        self.write_footer()
        return self.out_stream.close()

    def flush(self):
        return self.out_stream.flush()
    
    def metadata(self, o):

        self.write(o)


    def write(self, o):
        type = o.CLASS_ID_FIELD_NUMBER
        size = o.ByteSize()
        if type == self.content_class_id:
            self.content_count += 1
        if type == self.metadata_class_id:
            if self.compressed:
                self.stop_compression()
            self.metadata_offsets.append(self.bytes_written)
        assert 0 <= size < HIGH_BIT, "Message size not in range!"
        assert 0 < type < HIGH_BIT, "Type ID not in range!"
        if type != self.content_class_id:
            self.out_stream.write(pack("<I", size | HIGH_BIT))
            self.out_stream.write(pack("<I", type))
            self.bytes_written += 8
        else:
            self.out_stream.write(pack("<I", size))
            self.bytes_written += 4
        self.out_stream.write(o.SerializeToString())
        self.bytes_written += size
        if type == self.metadata_class_id:
            if self.compression:
                self.start_compression()


class A4InputStream(object):
    def __init__(self, in_stream):
        self.in_stream = in_stream
        self._orig_in_stream = in_stream
        self._eof = False
        self.size = 0
        self.headers = {}
        self.footers = {}
        self.metadata = {}
        self.read_header()
        self.current_header = self.headers.values()[0]
        self._read_all_meta_info = False
        self._metadata_change = True
        self.current_metadata = None


    def read_all_meta_info(self):
        if self._read_all_meta_info:
            return
        cached_state = self._orig_in_stream.tell(), self.in_stream
        self.in_stream = self._orig_in_stream
        self.headers = {}
        self.footers = {}
        self.metadata = {}
        self.size = 0
        while self.read_footer(self.size, read_metadata=True):
            print "SIZE IS ", self.size
            self.in_stream.seek(-self.size, SEEK_END)
            print "TELL IS ", self.in_stream.tell()
            self.read_header()
            first = False
            self.in_stream.seek(-self.size, SEEK_END)
            if self.in_stream.tell() == 0:
                break
        pos, self.in_stream = cached_state
        self._orig_in_stream.seek(pos)

    def get_header_at(self, position):
        hkeys = sorted(self.headers.keys())
        i = bisect(hkeys, position)
        print "HEADER AT ", hkeys, position, i
        return self.headers[hkeys[i-1]]

    def get_metadata_at(self, position):
        fw = self.get_header_at(position).metadata_refers_forward
        if fw:
            mkeys = sorted(self.metadata.keys())
            i = bisect(mkeys, position)
            if i == 0:
                print "AAAh ", position, mkeys

                return None
            return self.metadata[mkeys[i-1]]
        self.read_all_meta_info()
        mkeys = sorted(self.metadata.keys())
        i = bisect(mkeys, position)
        print "bisect result: ", mkeys, position, i
        if i == len(mkeys):
            return None
        return self.metadata[mkeys[i]]

    def __iter__(self):
        return self

    def read_header(self):
        # get the beginning-of-stream information
        magic = self.in_stream.read(len(START_MAGIC))
        assert START_MAGIC == magic, "Not an A4 file: Magic %s!" % magic
        header_position = self.in_stream.tell()
        cls, header = self.read_message()
        self.process_header(header, header_position)

    def process_header(self, header, header_position):
        assert header.a4_version == 1, "Incompatible stream version :( Upgrade your client?"
        if header.content_class_id:
            self.content_class_id = header.content_class_id
        if header.metadata_class_id:
            self.metadata_class_id = header.metadata_class_id
        self.headers[header_position] = header
        print "header at ", header_position, " is ", header

    def read_footer(self, neg_offset=0, read_metadata=True):
        self.in_stream.seek( - neg_offset - len(END_MAGIC), SEEK_END)
        if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
            print("File seems to be not closed!")
            self.in_stream.seek(0)
            return False
        self.in_stream.seek(-neg_offset - len(END_MAGIC) - 4, SEEK_END)
        footer_size, = unpack("<I", self.in_stream.read(4))
        footer_start = - neg_offset - len(END_MAGIC) - 4 - footer_size - 8
        self.in_stream.seek(footer_start, SEEK_END)
        footer_abs_start = self.in_stream.tell()
        cls, footer = self.read_message()
        print "FOOTER SIZE = 20 + ", footer_size
        print "FOOTER START = ", footer_start
        self.process_footer(footer, len(END_MAGIC) + 4 + footer_size + 8, self.in_stream.tell())
        # get metadata from footer
        if read_metadata:
            for metadata in footer.metadata_offsets:
                metadata_start = footer_abs_start - footer.size + metadata
                print "SEEKING TO ", metadata_start, footer_abs_start, footer.size, metadata
                self.in_stream.seek(metadata_start)
                cls, metadata = self.read_message()
                self.metadata[metadata_start] = metadata
                print "metadata at ", metadata_start, " is ", metadata
        return True

    def process_footer(self, footer, footer_size, footer_end):
        footer_start = footer_end - footer_size
        if not footer_start in self.footers:
            self.size += footer.size + footer_size
        self.footers[footer_start] = footer
        print "footer at ", footer_start, " is ", footer

    def info(self):
        self.read_all_meta_info()
        info = []
        version = [h.a4_version for h in self.headers.values()]
        info.append("A4 file v%i" % version[0])
        info.append("size: %s bytes" % self.size)
        hf = zip(self.headers.values(), self.footers.values())
        cccd = {}
        for c, cc in ((h.description, f.content_count) for h, f in hf):
            cccd[c] = cccd.get(c, 0) + cc
        for content in sorted(cccd.keys()):
            cc = cccd[content]
            if cc != 1:
                ms = "s"
            else:
                ms = ""
            info.append("%i %s%s" % (cccd[content], content, ms))
        return ", ".join(info)

    def read_message(self):
        size, = unpack("<I", self.in_stream.read(4))
        if size & HIGH_BIT:
            size = size & (HIGH_BIT - 1)
            type,  = unpack("<I", self.in_stream.read(4))
        else:
            type = self.content_class_id
        cls = class_ids[type]
        return cls, cls.FromString(self.in_stream.read(size))

    def next(self):
        cls, message = self.read_message()
        print "READ NEXT ", cls, message, " AT ", self.in_stream.tell()
        if cls is A4StreamHeader:
            self.process_header(message, self.in_stream.tell() - 8 - message.ByteSize())
            self.current_header = message
            return self.next()
        elif cls is A4StreamFooter:
            self.process_footer(message, len(END_MAGIC) + 4 + message.ByteSize() + 8, self.in_stream.tell())
            footer_size,  = unpack("<I", self.in_stream.read(4))
            if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
                print("File seems to be not closed!")
                self._eof = True
                raise StopIteration
            self.current_metadata = None
            self._metadata_change = True
            if not START_MAGIC == self.in_stream.read(len(START_MAGIC)):
                self._eof = True
                raise StopIteration
            return self.next()
        elif cls is A4StartCompressedSection:
            self._orig_in_stream = self.in_stream
            self.in_stream = ZlibInputStream(self._orig_in_stream)
            return self.next()
        elif cls is A4EndCompressedSection:
            self.in_stream.close()
            self.in_stream = self._orig_in_stream
            return self.next()
        elif cls.CLASS_ID_FIELD_NUMBER == self.current_header.metadata_class_id:
            self.metadata[self.in_stream.tell() - message.ByteSize() - 8] = message
            self._metadata_change = True
            if self.current_header.metadata_refers_forward:
                self.current_metadata = message
            else:
                self.current_metadata = self.get_metadata_at(self.in_stream.tell())
            print "FOUND CURRENT METADATA"
            return self.next()
        if not self.current_metadata:
            self.current_metadata = self.get_metadata_at(self.in_stream.tell())
        return message

    def itermetadata(self):
        class eventiter(object):
            def __init__(it, first, meta):
                it.first = first
                it.meta = meta
            def __iter__(it):
                return it
            def next(it):
                if it.first:
                    f = it.first
                    it.first = None
                    return f
                n = self.next()
                if self._metadata_change:
                    it.meta.next_first = n
                    raise StopIteration
                return n

        class metaiter(object):
            def __init__(it):
                it.next_first = None
            def __iter__(it):
                return it
            def next(it):
                if self._eof:
                    raise StopIteration
                assert self._metadata_change, "Cannot skip events in iteration over metadata!"
                if not it.next_first:
                    it.next_first = self.next()
                self._metadata_change = False
                return self.current_metadata, eventiter(it.next_first, it)
        return metaiter()

    def close(self):
        return self.in_stream.close()

    @property
    def closed(self):
        return self.in_stream.closed

def test_read(fn, n_events):
    """
    Test different modes of reading events
    assert that n_events
    """

    print "READ TEST NOSEEK"
    r = A4InputStream(file(fn))
    cnt = 0
    for e in r:
        print "Event: ", e
        cnt += 1
        print "Current Metadata: ", r.current_metadata
        assert r.current_metadata.meta_data == e.event_number//1000
    del r
    assert cnt == n_events

    print "READ TEST NOSEEK BY METADATA"
    cnt = 0
    r = A4InputStream(file(fn))
    for md, events in r.itermetadata():
        print "ITER METADATA: ", md
        for e in events:
            cnt += 1
            print "Event: ", e
            assert md.meta_data == e.event_number//1000
    assert cnt == n_events

    print "READ TEST SEEK"
    r = A4InputStream(file(fn))
    r.info()
    cnt = 0
    for e in r:
        cnt += 1
        print "Event: ", e
        print "Current Metadata: ", r.current_metadata
        assert r.current_metadata.meta_data == e.event_number//1000
    del r
    assert cnt == n_events

    print "READ TEST SEEK BY METADATA"
    r = A4InputStream(file(fn))
    r.info()
    cnt = 0
    for md, events in r.itermetadata():
        print "ITER METADATA: ", md
        for e in events:
            print "Event: ", e
            cnt += 1
            assert md.meta_data == e.event_number//1000

    assert cnt == n_events

def test_rw_forward(fn):
    from a4.proto.io import TestEvent, TestMetaData
    w = A4OutputStream(file(fn,"w"), "TestEvent", TestEvent, TestMetaData, True, metadata_refers_forward=True)
    e = TestEvent()
    m = TestMetaData()
    m.meta_data = 1
    w.write(m)
    for i in range(500):
        e.event_number = 1000+i
        w.write(e)
    m.meta_data = 2
    w.write(m)
    for i in range(500):
        e.event_number = 2000+i
        w.write(e)
    w.close()


def test_rw_backward(fn):
    from a4.proto.io import TestEvent, TestMetaData
    w = A4OutputStream(file(fn,"w"), "TestEvent", TestEvent, TestMetaData, True)
    e = TestEvent()
    for i in range(500):
        e.event_number = 1000+i
        w.write(e)
    m = TestMetaData()
    m.meta_data = 1
    w.write(m)
    for i in range(500):
        e.event_number = 2000+i
        w.write(e)
    m.meta_data = 2
    w.write(m)
    w.close()


def test_rw():
    from os import system
    print "-"*50
    print "Test Forward"
    print "-"*50
    test_rw_forward("pytest_fw.a4")
    test_read("pytest_fw.a4", 1000)
    print "-"*50
    print "Test Backward"
    print "-"*50
    test_rw_backward("pytest_bw.a4")
    test_read("pytest_bw.a4", 1000)
    system("cat pytest_fw.a4 pytest_fw.a4 > pytest_fwfw.a4")
    system("cat pytest_fw.a4 pytest_bw.a4 > pytest_fwbw.a4")
    system("cat pytest_bw.a4 pytest_bw.a4 > pytest_bwbw.a4")
    system("cat pytest_bw.a4 pytest_fw.a4 > pytest_bwfw.a4")
    print "-"*50
    print "Test Forward-Forward"
    print "-"*50
    test_read("pytest_fwfw.a4", 2000)
    print "-"*50
    print "Test Forward-Backward"
    print "-"*50
    test_read("pytest_fwbw.a4", 2000)
    print "-"*50
    print "Test Backward-Forward"
    print "-"*50
    test_read("pytest_bwfw.a4", 2000)
    print "-"*50
    print "Test Backward-Backward"
    print "-"*50
    test_read("pytest_bwbw.a4", 2000)


