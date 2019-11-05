## Usage

1. Create virtual environment
> virtualenv venv

2. Activate virtual env
> source venv/bin/activate

3. Install dependencies
> pip install -r requirements.txt

4. Analyze recorded traces
> python access_info.py /path/to/access_trace/folder /path/to/binary

The script should display the number of accesses for source code locations with their memory level.

For example:
```
Access with Memory Levels
Source Code     Level           Number of Accesses
matrix.c:54
                LOC_RAM         1
                L2              24
                LFB             14
                L3              143
                L1              1266046
matrix.c:52
                LFB             2
                L2              2
                L1              32165
matrix.c:100
                L1              9881
matrix.c:51
                L1              532
matrix.c:98
                L1              94
matrix.c:49
                L1              93
matrix.c:93
                L1              71
matrix.c:94
                L1              34
matrix.c:92
                L1              1
```
