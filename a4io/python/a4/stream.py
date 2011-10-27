from struct import pack, unpack
from bisect import bisect_right as bisect

from google.protobuf.message import Message
from google.protobuf.descriptor import Descriptor, FileDescriptor, FieldDescriptor, EnumDescriptor, EnumValueDescriptor
from google.protobuf.descriptor_pb2 import FileDescriptorProto

from a4.io.A4Stream_pb2 import StreamHeader, StreamFooter, StartCompressedSection, EndCompressedSection, ProtoClass

from .zlib_stream import ZlibInputStream, ZlibOutputStream
from .snappy_stream import SnappyInputStream
from .proto_class_pool import ProtoClassPool

START_MAGIC = "A4STREAM"
END_MAGIC = "KTHXBYE4"
HIGH_BIT = (1<<31)
SEEK_END = 2

class OutputStream(object):
    def __init__(self, out_stream, description=None, content_cls=None, metadata_cls=None, compression=True, metadata_refers_forward=False):
        self.compression = compression
        self.compressed = False
        self.bytes_written = 0
        self._raw_out_stream = out_stream
        self.out_stream = out_stream
        self.metadata_refers_forward = metadata_refers_forward
        self._written_class_ids = set()
        self._written_fdps = set()
        self.metadata_offsets = []
        self.protoclass_offsets = []
        self.next_class_id = 0
        self.next_metadata_class_id = 1
        self.class_id_map = {}

        self.write_header(description)
        self.start_compression()


    def start_compression(self):
        assert not self.compressed
        sc = StartCompressedSection()
        sc.compression = sc.ZLIB
        self.write(sc)
        self._pre_compress_bytes_written = self.bytes_written
        self.out_stream = ZlibOutputStream(self._raw_out_stream)
        self.compressed = True
    
    def stop_compression(self):
        assert self.compressed
        sc = EndCompressedSection()
        self.write(sc)
        self.out_stream.close()
        self.bytes_written = self.out_stream.bytes_written + self._pre_compress_bytes_written
        self.out_stream = self._raw_out_stream
        self.compressed = False

    def write_header(self, description=None):
        self.out_stream.write(START_MAGIC)
        self.bytes_written += len(START_MAGIC)
        header = StreamHeader()
        header.a4_version = 2
        header.metadata_refers_forward = self.metadata_refers_forward
        if description:
            header.description = description
        self.write(header)

    def write_footer(self):
        footer = StreamFooter()
        footer.size = self.bytes_written
        footer.metadata_offsets.extend(self.metadata_offsets)
        footer.protoclass_offsets.extend(self.protoclass_offsets)
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

    def get_descriptors_recursively(self, file_descriptor, file_descriptors=None, all_fds=None):
        if file_descriptors is None:
            file_descriptors = []
        if all_fds is None:
            import gc
            all_fds = [fd for fd in gc.get_objects() if isinstance(fd, FileDescriptor)]
        # Do always recurse into dependencies to assure that dependencies are written first
        file_descriptors.append(file_descriptor)
        fdp = FileDescriptorProto()
        file_descriptor.CopyToProto(fdp)
        for dep in fdp.dependency:
            matches = [fd for fd in all_fds if fd.name == dep]
            if len(matches) == 0:
                raise RuntimeError("MISSING A4 DEPENDENCY: %s" % dep)
            else:
                fd, = matches
                self.get_descriptors_recursively(fd, file_descriptors, all_fds)
        return file_descriptors

    def get_file_descriptor_protos(self, file_descriptors):
        fdps = []
        for fd in file_descriptors:
            fdp = FileDescriptorProto()
            fd.CopyToProto(fdp)
            if fdp.name in self._written_fdps:
                continue
            self._written_fdps.add(fdp.name)
            fdps.append(fdp)
        return fdps

    def write_proto_class(self, o, class_id):
        file_descriptors = self.get_descriptors_recursively(o.DESCRIPTOR.file)
        file_descriptors.reverse()
        protoclass = ProtoClass();
        protoclass.file_descriptor.extend(self.get_file_descriptor_protos(file_descriptors))
        protoclass.class_id = class_id
        protoclass.full_name = ".".join((o.DESCRIPTOR.file.package, o.DESCRIPTOR.name)).strip(".")
        class_id = self.get_class_id(protoclass, metadata=False)
        if self.compressed:
            self.stop_compression()
        self.protoclass_offsets.append(self.bytes_written)
        if self.compression:
            self.start_compression()
        self.write_message(class_id, protoclass)

    def get_class_id(self, o, metadata):
        try:
            class_id = self.class_id_map[o.__class__]
        except KeyError:
            class_id = ProtoClassPool.static_class_id(o.__class__)
            if class_id is None:
                if metadata:
                    class_id = self.next_metadata_class_id
                    self.next_metadata_class_id += 1
                else:
                    class_id = self.next_class_id
                    self.next_class_id += 1
                self.write_proto_class(o, class_id)
            self.class_id_map[o.__class__] = class_id
        return class_id

    def metadata(self, o):
        class_id = self.get_class_id(o, metadata=True)
        if self.compressed:
            self.stop_compression()
        self.metadata_offsets.append(self.bytes_written)
        if self.compression:
            self.start_compression()
        self.write_message(class_id, o)


    def write(self, o):
        class_id = self.get_class_id(o, metadata=False)
        self.write_message(class_id, o)

    def write_message(self, class_id, o):
        size = o.ByteSize()
        assert 0 <= size < HIGH_BIT, "Message size not in range!"
        assert 0 <= class_id < HIGH_BIT, "Class ID not in range: %s" %class_id
        if class_id != 0:
            self.out_stream.write(pack("<I", size | HIGH_BIT))
            self.out_stream.write(pack("<I", class_id))
            self.bytes_written += 8
        else:
            self.out_stream.write(pack("<I", size))
            self.bytes_written += 4
        self.out_stream.write(o.SerializeToString())
        self.bytes_written += size


class InputStream(object):
    def __init__(self, in_stream):
        self.in_stream = in_stream
        self._orig_in_stream = in_stream
        self._eof = False
        self.size = 0
        self.headers = {}
        self.footers = {}
        self.metadata = {}
        self._read_all_meta_info = False
        self.pool = ProtoClassPool()

        self.read_header()
        self.current_header = self.headers.values()[0]
        self._metadata_change = True
        self.current_metadata = None


    def read_all_meta_info(self):
        if self._read_all_meta_info:
            return
        self._read_all_meta_info = True
        cached_state = self._orig_in_stream.tell(), self.in_stream
        self.in_stream = self._orig_in_stream
        self.headers = {}
        self.footers = {}
        self.metadata = {}
        self.size = 0
        while self.read_footer(self.size, read_metadata=True):
            #print "SIZE IS ", self.size
            self.in_stream.seek(-self.size, SEEK_END)
            #print "TELL IS ", self.in_stream.tell()
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
        #print "HEADER AT ", hkeys, position, i
        return self.headers[hkeys[i-1]]

    def get_metadata_at(self, position):
        fw = self.get_header_at(position).metadata_refers_forward
        if fw:
            mkeys = sorted(self.metadata.keys())
            i = bisect(mkeys, position)
            if i == 0:
                #print "AAAh ", position, mkeys

                return None
            return self.metadata[mkeys[i-1]]
        self.read_all_meta_info()
        mkeys = sorted(self.metadata.keys())
        i = bisect(mkeys, position)
        #print "bisect result: ", mkeys, position, i
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
        cls, header, meta = self.read_message()
        self.process_header(header, header_position)

    def process_header(self, header, header_position):
        assert header.a4_version == 2, "Incompatible stream version :( Upgrade your client?"
        self.headers[header_position] = header
        #print "header at ", header_position, " is ", header

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
        cls, footer, meta = self.read_message()
        #print "FOOTER SIZE = 20 + ", footer_size
        #print "FOOTER START = ", footer_start
        self.process_footer(footer, len(END_MAGIC) + 4 + footer_size + 8, self.in_stream.tell())
        # get metadata from footer
        if read_metadata:
            for protoclass in sorted(footer.protoclass_offsets):
                protoclass_start = footer_abs_start - footer.size + protoclass
                self.in_stream.seek(protoclass_start)
                cls, protoclass, meta = self.read_message()
                self.pool.report_proto_class(protoclass)
                self.drop_compression()
            for metadata in sorted(footer.metadata_offsets):
                metadata_start = footer_abs_start - footer.size + metadata
                #print "SEEKING TO ", metadata_start, footer_abs_start, footer.size, metadata
                self.in_stream.seek(metadata_start)
                cls, metadata, meta = self.read_message()
                assert meta
                self.metadata[metadata_start] = metadata
                self.drop_compression()
                #print "metadata at ", metadata_start, " is ", metadata
        return True

    def process_footer(self, footer, footer_size, footer_end):
        footer_start = footer_end - footer_size
        if not footer_start in self.footers:
            self.size += footer.size + footer_size
        self.footers[footer_start] = footer
        #print "footer at ", footer_start, " is ", footer

    def info(self):
        def cname(id):
            cls = self.pool.find_class_id(id)
            if cls is None:
                return "<unknow class %i>" % id
            else:
                return self.pool.class_ids[id].__name__
        self.read_all_meta_info()
        info = []
        version = [h.a4_version for h in self.headers.values()]
        info.append("A4 file v%i" % version[0])
        info.append("size: %s bytes" % self.size)
        info.append("description: %s" % self.headers.values()[0].description)
        hf = zip(self.headers.values(), self.footers.values())
        return ", ".join(info)

    def drop_compression(self):
        self.in_stream.close()
        self.in_stream = self._orig_in_stream

    def read_message(self):
        size, = unpack("<I", self.in_stream.read(4))
        if size & HIGH_BIT:
            size = size & (HIGH_BIT - 1)
            class_id,  = unpack("<I", self.in_stream.read(4))
        else:
            class_id = 0
        cls = self.pool.find_class_id(class_id)

        #print "READ NEXT ", cls, " AT ", self.in_stream.tell()
        msg = cls.FromString(self.in_stream.read(size))

        if cls == StartCompressedSection:
            self._orig_in_stream = self.in_stream
            if msg.compression is msg.ZLIB:
                self.in_stream = ZlibInputStream(self._orig_in_stream)
            elif msg.compression is msg.SNAPPY:

                self.in_stream = SnappyInputStream(self._orig_in_stream)
            else:
                raise RuntimeError("Unknown compression in input: %s" % str(msg))
            return self.read_message()
        elif cls == EndCompressedSection:
            self.in_stream.close()
            self.in_stream = self._orig_in_stream
            return self.read_message()
        return cls, msg, (class_id%2==1)#metadata has odd class_ids

    def next(self):
        c, message, metadata = self.read_message()
        #print "READ NEXT ", c, " Metadata: ", metadata, " AT ", self.in_stream.tell()
        #print "READ NEXT ", cls, message, " AT ", self.in_stream.tell()
        if c == StreamHeader:
            self.process_header(message, self.in_stream.tell() - 8 - message.ByteSize())
            self.current_header = message
            return self.next()
        elif c == StreamFooter:
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
        elif c == ProtoClass:
            self.pool.report_proto_class(message)
            return self.next()
        elif metadata:
            self.metadata[self.in_stream.tell() - message.ByteSize() - 8] = message
            self._metadata_change = True
            if self.current_header.metadata_refers_forward:
                self.current_metadata = message
            else:
                self.current_metadata = self.get_metadata_at(self.in_stream.tell())
            #print "FOUND CURRENT METADATA"
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
    r = InputStream(file(fn))
    cnt = 0
    for e in r:
        #print "Event: ", e
        cnt += 1
        #print "Current Metadata: ", r.current_metadata
        assert r.current_metadata.meta_data == e.event_number//1000
    del r
    assert cnt == n_events

    print "READ TEST NOSEEK BY METADATA"
    cnt = 0
    r = InputStream(file(fn))
    for md, events in r.itermetadata():
        #print "ITER METADATA: ", md
        for e in events:
            cnt += 1
            #print "Event: ", e
            assert md.meta_data == e.event_number//1000
    assert cnt == n_events

    print "READ TEST SEEK"
    r = InputStream(file(fn))
    r.info()
    cnt = 0
    for e in r:
        cnt += 1
        #print "Event: ", e
        #print "Current Metadata: ", r.current_metadata
        assert r.current_metadata.meta_data == e.event_number//1000
    del r
    assert cnt == n_events

    print "READ TEST SEEK BY METADATA"
    r = InputStream(file(fn))
    r.info()
    cnt = 0
    for md, events in r.itermetadata():
        #print "ITER METADATA: ", md
        for e in events:
            #print "Event: ", e
            cnt += 1
            assert md.meta_data == e.event_number//1000

    assert cnt == n_events

def test_rw_forward(fn):
    from a4.io.A4Stream_pb2 import TestEvent, TestMetaData
    w = OutputStream(file(fn,"w"), "TestEvent", TestEvent, TestMetaData, True, metadata_refers_forward=True)
    e = TestEvent()
    m = TestMetaData()
    m.meta_data = 1
    w.metadata(m)
    for i in range(500):
        e.event_number = 1000+i
        w.write(e)
    m.meta_data = 2
    w.metadata(m)
    for i in range(500):
        e.event_number = 2000+i
        w.write(e)
    w.close()


def test_rw_backward(fn):
    from a4.io.A4Stream_pb2 import TestEvent, TestMetaData
    w = OutputStream(file(fn,"w"), "TestEvent", TestEvent, TestMetaData, True)
    e = TestEvent()
    for i in range(500):
        e.event_number = 1000+i
        w.write(e)
    m = TestMetaData()
    m.meta_data = 1
    w.metadata(m)
    for i in range(500):
        e.event_number = 2000+i
        w.write(e)
    m.meta_data = 2
    w.metadata(m)
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


