

def test_writing():
    from a4 import A4WriterStream
    w = A4WriterStream(open("test.a4", "w"), content="Event")
    from a4.A4Stream import A4StartCompressedSection
    from a4.A4Stream import A4EndCompressedSection
    s =A4StartCompressedSection()
    s.compression = s.BZIP2
    w.write(s)
    s = A4EndCompressedSection()
    s.size = 42
    w.write(s)
    w.close()

def test_reading():
    from a4 import A4ReaderStream
    r = A4ReaderStream(open("test.a4"))
    msg1 = r.read()
    assert msg1.compression == msg1.BZIP2
    msg2 = r.read()
    assert msg2.size == 42
    assert r.read() is None
    assert r.info()
