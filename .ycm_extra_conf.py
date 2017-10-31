import os

def FlagsForFile(filename, **kwargs):

    flags = ['-std=c++11', '-I/usr/local/include']
    return {'flags': flags}

