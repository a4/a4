from io import IOBase
from struct import pack, unpack

from messages.A4Stream_pb2 import A4StreamHeader, A4StreamFooter, A4StartCompressedSection, A4EndCompressedSection

numbering = dict((cls.CLASS_ID_FIELD_NUMBER, cls) for cls in 
                 (A4StreamHeader, A4StreamFooter, A4StartCompressedSection, A4EndCompressedSection))

START_MAGIC = "A4STREAM"
END_MAGIC = "KTHXBYE4"
HIGH_BIT = (1<<31)
SEEK_END = 2

class A4WriterStream(IOBase):
    def __init__(self, out_stream, content=None):
        self.bytes_written = 0
        self.content_count = 0
        self.previous_type = 0
        self.out_stream = out_stream
        self.out_stream.write(START_MAGIC)
        self.bytes_written += len(START_MAGIC)
        header = A4StreamHeader()
        header.a4_version = 1
        if content:
            header.content = content
        self.write(header)

    def close(self):
        footer = A4StreamFooter()
        footer.size = self.bytes_written
        footer.content_count = self.content_count
        self.write(footer)
        self.out_stream.write(pack("!I", footer.ByteSize()))
        self.out_stream.write(END_MAGIC)
        self.bytes_written += len(END_MAGIC)
        return self.out_stream.close()

    @property
    def closed(self):
        return self.out_stream.closed
    
    def flush(self):
        return self.out_stream.flush()
    
    def write(self, o):
        type = o.CLASS_ID_FIELD_NUMBER
        size = o.ByteSize()
        assert 0 <= size < HIGH_BIT, "Message size not in range!"
        assert 0 < type < HIGH_BIT, "Type ID not in range!"
        if type != self.previous_type:
            self.out_stream.write(pack("!I", size | HIGH_BIT))
            self.out_stream.write(pack("!I", type))
            self.bytes_written += 8
            self.previous_type = type
        else:
            self.out_stream.write(pack("!I", size))
            self.bytes_written += 4
        self.out_stream.write(o.SerializeToString())
        self.bytes_written += size

class A4ReaderStream(IOBase):
    def __init__(self, in_stream):
        self.previous_type = None
        self.in_stream = in_stream
        self.size = 0
        self.headers = []
        # get the beginning-of-stream information
        assert START_MAGIC == self.in_stream.read(len(START_MAGIC)), "Not an A4 file!"
        cls, header = self.read_message()
        assert header.a4_version == 1, "Incompatible stream version :( Upgrade your client?"
        self.header = header
        # get the end-of-stream information
        self.in_stream.seek(-len(END_MAGIC), SEEK_END)
        if not END_MAGIC == self.in_stream.read(len(END_MAGIC)):
            print("File seems to be not closed! Continuing anyway...")
            self.in_stream.seek(0)
            return     
        self.in_stream.seek(-len(END_MAGIC)-4, SEEK_END)
        footer_size,  = unpack("!I", self.in_stream.read(4))
        footer_start = -len(END_MAGIC) - 4 - footer_size - 8
        self.in_stream.seek(footer_start, SEEK_END)
        cls, footer = self.read_message()
        self.footer = footer
        self.size = footer.size - footer_start
        # Try to find if this is a concatenated file
        self.in_stream.seek(-self.size, SEEK_END)
        if not self.in_stream.tell() == 0:
            print("WARNING: concatenating files not supported yet")
        self.in_stream.seek(len(START_MAGIC))

    def info(self):
        info = []
        info.append("A4 file v%i" % self.header.a4_version)
        info.append("size: %s bytes" % self.size)
        if self.header.content:
            if self.footer:
                c = self.footer.content_count
                info.append("content: %i %s%s" % (c, self.header.content, "s" if c != 1 else ""))
            else:
                info.append("content: %s" % (self.header.content))
        return ", ".join(info)

    def read_message(self):
        size, = unpack("!I", self.in_stream.read(4))
        if size & HIGH_BIT:
            size = size & (HIGH_BIT - 1)
            type,  = unpack("!I", self.in_stream.read(4))
            self.previous_type = type
        else:
            type = self.previous_type
        cls = numbering[type]
        return cls, cls.FromString(self.in_stream.read(size))

    def read(self):
        cls, message = self.read_message()
        if cls is A4StreamHeader:
            self.headers.append(message)
            self.header = message
            return self.read()
        if cls is A4StreamFooter:
            footer_size,  = unpack("!I", self.in_stream.read(4))
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
