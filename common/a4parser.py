#! /usr/bin/env python
"""
Experiemental (incomplete) hachoir A4 event stream parser
"""

from hachoir_core.cmd_line import (getHachoirOptions,
    configureHachoir, unicodeFilename)
    
from hachoir_parser import Parser
from hachoir_core.field import (
    GenericInteger, UInt8, UInt16, UInt32, Enum, TimestampUnix32,
    Bit, Bits, CString, SubFile, FieldSet,
    NullBits, Bytes, RawBytes)
from hachoir_core.text_handler import textHandler, hexadecimal, filesizeHandler
from hachoir_core.endian import LITTLE_ENDIAN
from hachoir_parser.common.deflate import Deflate

def _(arg): return arg

from optparse import OptionGroup, OptionParser

class VarInt(Bits):
    def createValue(self):
        while True:
            self._parent.stream.readInteger(self.absolute_address, False, 8, self._parent.endian)
        

class ProtobufField(FieldSet):
    wire_type = {
        0: "VARINT",
        1: "FIXED64",
        2: "LENGTH_DELIMITED",
        3: "START_GROUP",
        4: "END_GROUP",
        5: "FIXED32",
    }
    def createFields(self):
        yield VarInt(self, "tag_and_type")

class ProtobufMessage(FieldSet):        
    def createFields(self):
        # TODO: Implementation
        while self.current_size < self.size:
            yield ProtobufField(self, "fields[]")

class A4Message(FieldSet):
    fixed_classes = {
        100: "StreamHeader",
        102: "StreamFooter",
        104: "StartCompressedSection",
        106: "EndCompressedSection",
        108: "ProtoClass",
    }    
    
    def createFields(self):
        yield GenericInteger(self, "size", False, 30)
        yield Bit(self, "reserved",        "reserved")
        yield Bit(self, "classid_follows", "next integer is classid")
        
        if self["classid_follows"]:
            yield Enum(UInt32(self, "class_id"), self.fixed_classes)
            
        yield ProtobufMessage(self, "contents", size=self["size"].value*8)


    
class A4Parser(Parser):
    endian = LITTLE_ENDIAN
    A4_OPEN_SIGNATURE = "A4STREAM"
    PARSER_TAGS = {
        "id": "a4",
        "category": "archive",
        "file_ext": ("a4",),
        "mime": (u"application/a4",),
        "min_size": 8,
        "magic": ((A4_OPEN_SIGNATURE, 0),),
        "description": u"A4 event data",
    }

    def validate(self):
        if self["signature"].value != A4_OPEN_SIGNATURE:
            return "Invalid signature"
        return True

    def createFields(self):
        yield Bytes(self, "signature", 8, "A4 file signature")
        
        #while not self.eof:
        yield A4Message(self, "message[]")
        
        size = (self._size - self.current_size) // 8
        print size, self._size, self.current_size
        yield RawBytes(self, "unparsed[]", size - 8)
        yield Bytes(self, "end_tag", 8, "End tag")
        
    def createDescription(self):
        desc = u"a4 archive"
        info = []
        if "filename" in self:
            info.append('filename "%s"' % self["filename"].value)
        if "size" in self:
            info.append("was %s" % self["size"].display)
        #if self["mtime"].value: info.append(self["mtime"].display)
        return "%s: %s" % (desc, ", ".join(info))

def parseOptions():
    parser = OptionParser(usage="%prog [options] filename")

    common = OptionGroup(parser, "Urwid", _("Option of urwid explorer"))
    common.add_option("--preload", help=_("Number of fields to preload at each read"),
        type="int", action="store", default=15)
    common.add_option("--path", help=_("Initial path to focus on"),
        type="str", action="store", default=None)
    common.add_option("--parser", help=_("Use the specified parser (use its identifier)"),
        type="str", action="store", default=None)
    common.add_option("--offset", help=_("Skip first bytes of input file"),
        type="long", action="store", default=0)
    common.add_option("--profiler", help=_("Run profiler"),
        action="store_true", default=False)
    common.add_option("--profile-display", help=_("Force update of the screen beetween each event"),
        action="store_true", default=False)
    common.add_option("--size", help=_("Maximum size of bytes of input file"),
        type="long", action="store", default=None)
    common.add_option("--hide-value", dest="display_value", help=_("Don't display value"),
        action="store_false", default=True)
    common.add_option("--hide-size", dest="display_size", help=_("Don't display size"),
        action="store_false", default=True)
    parser.add_option_group(common)

    hachoir = getHachoirOptions(parser)
    parser.add_option_group(hachoir)

    values, arguments = parser.parse_args()
    if len(arguments) != 1:
        parser.print_help()
        sys.exit(1)
    return values, arguments[0]
    
def main():
    
    from hachoir_core.stream import InputStreamError, FileInputStream
    from hachoir_urwid import exploreFieldSet
    
    values, filename = parseOptions()
    configureHachoir(values)
    
    from sys import argv
    instream = FileInputStream(unicodeFilename(filename))
    parser = A4Parser(instream)
    #parser.parse(instream)
    
    exploreFieldSet(parser, values)

if __name__ == "__main__":
    main()
