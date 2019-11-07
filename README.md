# MemAccessTrace

The header-only library provides an interface to store memory access events in a trace file.
It provides an interface for writing and reading traces.
Currently, *MemAccessTrace* intends to create one trace per thread. 

*MemAccessTrace* also provides a Python interface for reading and writing trace files.
Take a look in the examples to see how trace files can be analyzed with Python.

This software was developed as part of the EC H2020 funded project NEXTGenIO (Project ID: 671951) http://www.nextgenio.eu.

# Usage
Examples can be examined in the `test` folder.

# Dependencies
* C++17
* Boost >= 1.69

# Planed Features
- [x] Python bindings
- [ ] Compression
- [ ] Concept for shared accesses
