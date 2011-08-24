from struct import pack, unpack

from messages.A4Stream_pb2 import A4StreamHeader, A4StreamFooter, A4StartCompressedSection, A4EndCompressedSection

numbering = dict((cls.CLASS_ID_FIELD_NUMBER, cls) for cls in 
                 (A4StreamHeader, A4StreamFooter, A4StartCompressedSection, A4EndCompressedSection))

START_MAGIC = "A4STREAM"
END_MAGIC = "KTHXBYE4"
HIGH_BIT = (1<<31)
SEEK_END = 2

class A4WriterStream(object):
    def __init__(self, out_stream, content=None, content_cls=None):
        self.bytes_written = 0
        self.content_count = 0
        self.content_class_id = None
        self.out_stream = out_stream
        self.write_header(content, content_cls)

    def write_header(self, content_name=None, content_cls=None):
        self.out_stream.write(START_MAGIC)
        self.bytes_written += len(START_MAGIC)
        header = A4StreamHeader()
        header.a4_version = 1
        if content_name:
            self.content_class_id = content_cls.CLASS_ID_FIELD_NUMBER
            header.content = content_name
            header.content_class_id = self.content_class_id
        else:
            self.content_class_id = None
        self.write(header)

    def write_footer(self, metadata=None):
        footer = A4StreamFooter()
        if metadata:
            self.write(metadata)
            meta_size = metadata.ByteSize()
            footer.metadata_offset = meta_size
        footer.size = self.bytes_written
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

    def close(self, metadata):
        self.write_footer(metadata)
        return self.out_stream.close()

    def flush(self):
        return self.out_stream.flush()
    
    def write(self, o):
        type = o.CLASS_ID_FIELD_NUMBER
        size = o.ByteSize()
        if type == self.content_class_id:
            self.content_count += 1
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


class A4ReaderStream(object):
    def __init__(self, in_stream):
        self.numbering = dict(numbering)
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
        else:
            self.content_class_id = 1001 # class id of Event for legacy files
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
        if footer.metadata_offset:
            metadata_start = footer_start - footer.metadata_offset - 8
            self.in_stream.seek(metadata_start, SEEK_END)
            cls, metadata = self.read_message()
            self.metadata[metadata_start] = metadata

        return True

    def register(self, cls):
        self.numbering[cls.CLASS_ID_FIELD_NUMBER] = cls

    def info(self):
        info = []
        version = [h.a4_version for h in self.headers.values()]
        info.append("A4 file v%i" % version[0])
        info.append("size: %s bytes" % self.size)
        hf = zip(self.headers.values(), self.footers.values())
        cccd = {}
        for c, cc in ((h.content, f.content_count) for h, f in hf):
            cccd[c] = cccd.get(c, 0) + cc
        for content in sorted(cccd.keys()):
            cc = cccd[content]
            if cc != 1:
                ms = "s"
            else:
                ms = ""
            info.append("%i %s%s" % (cccd[content], content, ms))

        #if self.metadata:
        #    info.append("\n")
        #for index in sorted(self.metadata):
        #    info.append("Metadata at %s\n" % index)
        #    info.append(str(self.metadata[index]))
        #    info.append("\n")
        return ", ".join(info)

    def read_message(self):
        size, = unpack("<I", self.in_stream.read(4))
        if size & HIGH_BIT:
            size = size & (HIGH_BIT - 1)
            type,  = unpack("<I", self.in_stream.read(4))
        else:
            type = self.content_class_id
        cls = self.numbering[type]
        return cls, cls.FromString(self.in_stream.read(size))

    def read(self):
        cls, message = self.read_message()
        if cls is A4StreamHeader:
            return self.read()
        if cls is A4StreamFooter:
            footer_size,  = unpack("<I", self.in_stream.read(4))
            if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
                print("File seems to be not closed!")
                return None
            if not START_MAGIC == self.in_stream.read(len(START_MAGIC)):
                return None
            return self.read()
        return message

    def close(self):
        return self.in_stream.close()

    @property
    def closed(self):
        return self.in_stream.closed
