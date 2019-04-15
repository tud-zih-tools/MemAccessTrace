#! /usr/bin/env python3
import unittest
import tracefile as tf

class TestAccessEvent(unittest.TestCase):
    def test_creation(self):
        a = tf.AccessEvent(1, 1, 42, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_L1)
        self.assertEqual(a.timestamp, 1)
        self.assertEqual(a.address, 1)
        self.assertEqual(a.ip, 42)
        self.assertEqual(a.type, tf.AccessType.LOAD)
        self.assertEqual(a.level, tf.MemoryLevel.MEM_LVL_L1)
        b = tf.AccessEvent()
        self.assertEqual(b.timestamp, 0)
        self.assertEqual(b.address, 0)
        self.assertEqual(b.ip, 0)
        self.assertEqual(b.type, tf.AccessType.NA)
        self.assertEqual(b.level, tf.MemoryLevel.MEM_LVL_NA)

class TestEventBuffer(unittest.TestCase):
    def test_add(self):
        buffer = tf.EventVectorBuffer()
        self.assertEqual(len(buffer), 0)
        buffer.append(tf.AccessEvent(1, 1, 1, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_HIT))
        self.assertEqual(len(buffer), 1)

    def test_range(self):
        buffer = tf.EventVectorBuffer()
        data = [
            tf.AccessEvent(1, 1, 1, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_HIT),
            tf.AccessEvent(2, 2, 2, tf.AccessType.STORE, tf.MemoryLevel.MEM_LVL_MISS),
            tf.AccessEvent(3, 3, 3, tf.AccessType.EXEC, tf.MemoryLevel.MEM_LVL_L1),
        ]

        for a in data:
            buffer.append(a)

        self.assertEqual(len(buffer), 3)

        for a, e in zip(buffer, data):
            self.assertEqual(a.timestamp, e.timestamp)
            self.assertEqual(a.address, e.address)
            self.assertEqual(a.ip, e.ip)
            self.assertEqual(a.type, e.type)
            self.assertEqual(a.level, e.level)

    def test_get_item(self):
        a1 = tf.AccessEvent(1, 1, 42, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_L1)
        a2 = tf.AccessEvent(2, 2, 44, tf.AccessType.STORE, tf.MemoryLevel.MEM_LVL_L2)
        buffer = tf.EventVectorBuffer()
        buffer.append(a1)
        buffer.append(a2)

        self.assertEqual(len(buffer), 2)
        self.assertEqual(a1.timestamp, buffer[0].timestamp)
        self.assertEqual(a1.address, buffer[0].address)
        self.assertEqual(a1.ip, buffer[0].ip)
        self.assertEqual(a1.type, buffer[0].type)
        self.assertEqual(a1.level, buffer[0].level)

        self.assertEqual(a2.timestamp, buffer[1].timestamp)
        self.assertEqual(a2.address, buffer[1].address)
        self.assertEqual(a2.ip, buffer[1].ip)
        self.assertEqual(a2.type, buffer[1].type)
        self.assertEqual(a2.level, buffer[1].level)

    def test_set_item(self):
        a1 = tf.AccessEvent(1, 1, 42, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_L1)
        a2 = tf.AccessEvent(2, 2, 44, tf.AccessType.STORE, tf.MemoryLevel.MEM_LVL_L2)
        buffer = tf.EventVectorBuffer()
        buffer.append(a1)
        buffer[0] = a2
        self.assertEqual(a2.timestamp, buffer[0].timestamp)
        self.assertEqual(a2.address, buffer[0].address)
        self.assertEqual(a2.ip, buffer[0].ip)
        self.assertEqual(a2.type, buffer[0].type)
        self.assertEqual(a2.level, buffer[0].level)

    class TestTraceMetaData(unittest.TestCase):
        def test_creation(self):
            buffer = tf.EventVectorBuffer()
            md = tf.TraceMetaData(buffer, 1337)
            self.assertEqual(md.size(), 0)
            self.assertEqual(md.thread_id(), 1337)

if __name__ == '__main__':
    unittest.main()