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
        buffer = tf.EventBuffer(100)
        self.assertEqual(len(buffer), 0)
        self.assertEqual(buffer.capacity(), 100)
        buffer.append(tf.AccessEvent(1, 1, 1, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_HIT))
        self.assertEqual(len(buffer), 1)
        self.assertEqual(buffer.capacity(), 100)

    def test_range(self):
        buffer = tf.EventBuffer(4)
        data = [
            [1, 1, 1, tf.AccessType.LOAD, tf.MemoryLevel.MEM_LVL_HIT],
            [2, 2, 2, tf.AccessType.STORE, tf.MemoryLevel.MEM_LVL_MISS],
            [3, 3, 3, tf.AccessType.EXEC, tf.MemoryLevel.MEM_LVL_L1],
        ]

        for a in data:
            buffer.append(a[0], a[1], a[2], a[3], a[4])

        self.assertEqual(len(buffer), 3)

        for a, e in zip(buffer, data):
            self.assertEqual(a.timestamp, e[0])
            self.assertEqual(a.address, e[1])
            self.assertEqual(a.ip, e[2])
            self.assertEqual(a.type, e[3])
            self.assertEqual(a.level, e[4])

if __name__ == '__main__':
    unittest.main()