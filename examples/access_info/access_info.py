#!/usr/bin/env python3
import pathlib
import collections
import operator
from typing import Callable
import elftools.common.py3compat as pc
from elftools.elf.elffile import ELFFile
import argparse

import tracefile as tf

class DwarfInfo:
    def __init__(self, exe):
        self.binary = exe
        self.fd = open(exe, 'rb')
        self.elf_file = ELFFile(self.fd)

        if not self.elf_file.has_dwarf_info():
            raise('Binary contains no dwarf info section.')
        self.dwarf_info = self.elf_file.get_dwarf_info()

    def __del__(self):
        self.fd.close()

    def lookup(self, address):
        # iterate over the compile units(CUs)
        for CU in self.dwarf_info.iter_CUs():
            line_progs = self.dwarf_info.line_program_for_CU(CU)
            prev_state = None
            # debug-line parse the table like `dwarfdump -debug-line ./main`
            for entry in line_progs.get_entries():
                if entry.state is None:
                    continue
                if entry.state.end_sequence:
                    prev_state = None
                    continue
                if prev_state and prev_state.address <= address < entry.state.address:
                    file_name = line_progs['file_entry'][prev_state.file - 1].name
                    line = prev_state.line
                    return file_name, line
                prev_state = entry.state
        raise('Could not find address')


def source_code_locations(binary, ips):
    dwarf = DwarfInfo(binary)
    valid_ips = dict()
    for ip in ips:
        try:
            file, line = dwarf.lookup(int(ip))
            valid_ips[int(ip)] =  "{}:{}".format(file.decode('utf-8'), line)
        except:
            pass
    return valid_ips


class SourceCodeLocation:
    def _valid_ips(self, new_ips: set):
        for ip in new_ips:
            try:
                file, line = self._dwarf.lookup(int(ip))
                self._ip_cache[int(ip)] =  "{}:{}".format(file.decode('utf-8'), line)
            except:
                pass


    def __init__(self, binary: str, eventbuffers: dict):
        self._binary = binary
        self._dwarf = DwarfInfo(binary)
        self._buffers = eventbuffers
        self._ip_cache = dict()


    def access_statistics(self, thread) -> list:
        ips = set(event.ip for event in self._buffers[thread])
        # No filtering if IP cache is empty
        if self._ip_cache:
            ips = filter(lambda ip: not ip in self._ip_cache, ips)
        # Update IP cache
        self._valid_ips(ips)
        # Count occurrence
        # access_hist = collections.defaultdict(int)
        access_hist = collections.defaultdict(lambda: collections.defaultdict(int))
        for event in self._buffers[thread]:
            if event.ip in self._ip_cache:
                level = str(event.level).replace("MemoryLevel.MEM_LVL_", "")
                access_hist[self._ip_cache.get(event.ip)][level] += 1
        return access_hist


def merge_dicts(a: dict, b: dict, f: Callable[[dict,dict], dict]) -> dict:
    # Add difference of dictionaries
    merged_dicts = {k: b.get(k, a.get(k)) for k in a.keys() ^ b.keys()}
    # Merge intersection entries
    merged_dicts.update({k: f(a.get(k), b.get(k)) for k in a.keys() & b.keys()})
    return merged_dicts


def merge_inner_dict(a: dict, b: dict):
    f = lambda c, d: c + d
    # Add difference of dictionaries
    merged_dicts = {k: b.get(k, a.get(k)) for k in a.keys() ^ b.keys()}
    # Merge intersection entries
    merged_dicts.update({k: f(a.get(k), b.get(k)) for k in a.keys() & b.keys()})
    return merged_dicts


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("accesstrace", help="Path to the access trace file i.e. trace.123.bin", type=str)
    parser.add_argument("binary", help="Path to the executable of the application i.e. /mnt/bin/matrix", type=str)
    args = parser.parse_args()

    exe = args.binary
    trace_path = pathlib.Path(args.accesstrace)

    traces = [entry for entry in trace_path.iterdir() if entry.is_file()]
    events = dict()
    mds = list()

    for t in traces:
        with tf.TraceFile(str(t), tf.TraceFileMode.READ) as file:
            eventbuffer, md = file.read()
            events[md.thread_id()] = eventbuffer
            mds.append(md)

    scl = SourceCodeLocation(exe, events)
    scl_stats = dict()
    for md in mds:
        thread_stat = scl.access_statistics(md.thread_id())
        scl_stats = merge_dicts(scl_stats, thread_stat, merge_inner_dict)

    result = sorted(scl_stats.items(), key=lambda x: sum(x[1].values()), reverse=True)

    print("\nAccess with Memory Levels")
    print("Source Code\tLevel\t\tNumber of Accesses")
    for code_location, level_stats in result:
        print("{}".format(code_location))
        for lvl, count in level_stats.items():
            print("\t\t{}\t\t{}".format(lvl, count))
