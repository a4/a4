from google.protobuf.internal.decoder import _DecodeVarint32

import ctypes as C
from commands import getstatusoutput
from os.path import join as pjoin


dll_names = ["libsnappy.so"]

s, out = getstatusoutput("pkg-config a4 --libs-only-L")
if s == 0:
    dll_names.extend(pjoin(l[len("-L"):], "libsnappy.so") for l in out.strip().split())

dll = None
for dll_name in dll_names:
    try:
        dll = C.CDLL(dll_name)
        break
    except OSError:
        pass

class SnappyDLL(object):
    def __init__(self, dll):
        #snappy_status snappy_compress(const char* input,
        #                              size_t input_length,
        #                              char* compressed,
        #                              size_t* compressed_length);
                                      
        self._compress = dll.snappy_compress
        self._compress.restype = C.c_int
        self._compress.argtypes = C.c_char_p, C.c_size_t, C.c_char_p, C.c_size_t

        #snappy_status snappy_uncompress(const char* compressed,
        #                                size_t compressed_length,
        #                                char* uncompressed,
        #                                size_t* uncompressed_length);

        self._uncompress = dll.snappy_uncompress
        self._uncompress.restype = C.c_int
        self._uncompress.argtypes = C.c_char_p, C.c_size_t, C.c_char_p, C.POINTER(C.c_size_t)

        #snappy_status snappy_uncompressed_length(const char* compressed,
        #                                         size_t compressed_length,
        #                                         size_t* result);

        self.uncompressed_length = dll.snappy_uncompressed_length
        self.uncompressed_length.argtypes = C.c_char_p, C.c_size_t, C.POINTER(C.c_size_t)

snappy_dll = None
if dll:
    snappy_dll = SnappyDLL(dll)

def uncompress(bytes):
    length = C.c_size_t()
    snappy_dll.uncompressed_length(bytes, len(bytes), C.byref(length))
    
    uncompressed = C.create_string_buffer(length.value)
    
    snappy_dll._uncompress(bytes, len(bytes), uncompressed, C.byref(length))
    
    return uncompressed[:length.value]

class SnappyOutputStream(object):
    """
    NOT YET TESTED
    """
    def __init__(self, out_stream):
        assert dll, "Snappy dynamic library could not be loaded!"
        self.out_stream = out_stream
        self.bytes_written = 0

    def write(self, data):
        compressed = self.dco.compress(data)
        self.bytes_written += len(compressed)
        self.out_stream.write(compressed)

    def close(self):
        self.bytes_written += len(compressed)
        self.out_stream.write(compressed)

class SnappyInputStream(object):
    """
    NOT YET TESTED
    """
    def __init__(self, in_stream):
        assert dll, "Snappy dynamic library could not be loaded!"
        self.in_stream = in_stream
        tell = self.in_stream.tell()
        self.tell = lambda : tell
        
        self.buffer = ""

    def read_block(self):
        
        TO_READ = 4
        content = self.in_stream.read(TO_READ)
        total_size, offset = _DecodeVarint32(content, 0)
        content = content[offset:]
        
        consumed = TO_READ - offset
        to_read = total_size - consumed
        
        if to_read > 0:
            content += self.in_stream.read(to_read)
        
        self.buffer += uncompress(content)
        
        

    def read(self, bytes):
        if not bytes:
            return ""
        while len(self.buffer) < bytes:
            self.read_block()
        
        self.buffer, result = self.buffer[bytes:], self.buffer[:bytes]
        return result

    def close(self):
        pass
