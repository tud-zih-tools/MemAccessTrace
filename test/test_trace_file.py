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

if __name__ == '__main__':
    unittest.main()