

def test_writing():
    from a4 import A4WriterStream
    from a4.A4Stream import A4StartCompressedSection, A4EndCompressedSection
    from a4.messages.Event_pb2 import Event
    w = A4WriterStream(open("test.a4", "w"), content="Event", content_cls = Event)
    s =A4StartCompressedSection()
    s.compression = s.BZIP2
    w.write(s)
    s = A4EndCompressedSection()
    s.size = 42
    w.write(s)
    e = Event()
    e.event_number = 1000
    e.mc_event_weight = 1.2
    w.write(e)
    e.event_number = 1001
    e.mc_event_weight = 1.1
    w.write(e)
    e.event_number = 1002
    e.mc_event_weight = 0.9
    w.write(e)
    w.close()

def test_reading():
    from a4 import A4ReaderStream
    from a4.messages.Event_pb2 import Event
    r = A4ReaderStream(open("test.a4"))
    r.register(Event)
    msg1 = r.read()
    assert msg1.compression == msg1.BZIP2
    msg2 = r.read()
    assert msg2.size == 42
    assert r.read().event_number == 1000
    assert r.read().event_number == 1001
    assert r.read().event_number == 1002
    assert r.read() is None
    assert r.info()
