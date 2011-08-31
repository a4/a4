from struct import pack, unpack
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

class A4WriterStream(object):
    def __init__(self, out_stream, description=None, content_cls=None, metadata_cls=None, compression=True):
        self.compression = True
        self.compressed = False
        self.bytes_written = 0
        self.content_count = 0
        self.content_class_id = None
        self._raw_out_stream = out_stream
        self.out_stream = out_stream
        self.content_class_id = None
        self.metadata_class_id = None
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


class A4ReaderStream(object):
    def __init__(self, in_stream):
        self.in_stream = in_stream
        self.size = 0
        self.headers = {}
        self.footers = {}
        self.metadata = {}
        self.read_header()
        while self.read_footer(self.size):
            self.in_stream.seek(-self.size, SEEK_END)
            self.read_header()
            first = False
            self.in_stream.seek(-self.size, SEEK_END)
            if self.in_stream.tell() == 0:
                break
        self.in_stream.seek(len(START_MAGIC))

    def read_header(self):
        # get the beginning-of-stream information
        header_position = self.in_stream.tell()
        assert START_MAGIC == self.in_stream.read(len(START_MAGIC)), "Not an A4 file!"
        cls, header = self.read_message()
        assert header.a4_version == 1, "Incompatible stream version :( Upgrade your client?"
        if header.content_class_id:
            self.content_class_id = header.content_class_id
        if header.metadata_class_id:
            self.metadata_class_id = header.metadata_class_id
        self.headers[header_position] = header

    def read_footer(self, neg_offset=0):
        self.in_stream.seek(-neg_offset - len(END_MAGIC), SEEK_END)
        if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
            print("File seems to be not closed!")
            self.in_stream.seek(0)
            return False
        self.in_stream.seek(-neg_offset - len(END_MAGIC) - 4, SEEK_END)
        footer_size, = unpack("<I", self.in_stream.read(4))
        footer_start = - neg_offset - len(END_MAGIC) - 4 - footer_size - 8
        self.in_stream.seek(footer_start, SEEK_END)
        cls, footer = self.read_message()
        self.footers[footer_start] = footer
        self.size += footer.size - footer_start - neg_offset

        # get metadata from footer
        for metadata in footer.metadata_offsets:
            metadata_start = metadata
            self.in_stream.seek(metadata_start)
            cls, metadata = self.read_message()
            self.metadata[metadata_start] = metadata

        return True

    def info(self):
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

    def read(self):
        cls, message = self.read_message()
        if cls is A4StreamHeader:
            return self.read()
        elif cls is A4StreamFooter:
            footer_size,  = unpack("<I", self.in_stream.read(4))
            if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
                print("File seems to be not closed!")
                return None
            if not START_MAGIC == self.in_stream.read(len(START_MAGIC)):
                return None
            return self.read()
        elif cls is A4StartCompressedSection:
            self._orig_in_stream = self.in_stream
            self.in_stream = ZlibInputStream(self._orig_in_stream)
            return self.read()
        elif cls is A4EndCompressedSection:
            self.in_stream.close()
            self.in_stream = self._orig_in_stream
        return message

    def close(self):
        return self.in_stream.close()

    @property
    def closed(self):
        return self.in_stream.closed


def test_rw():
    from a4.proto.io import TestEvent, TestMetaData
    fn = "pytest.a4"
    w = A4WriterStream(file(fn,"w"), "TestEvent", TestEvent, TestMetaData, True)
    e = TestEvent()
    for i in range(100):
        e.event_number = i
        w.write(e)
    m = TestMetaData()
    m.meta_data = 1
    w.write(m)
    for i in range(100):
        e.event_number = 200+i
        w.write(e)
    m.meta_data = 2
    w.write(m)
    w.close()

