'''
Check if the files in a directory conform to whitespace rules
'''
import os
from os.path import join

TYPES_TO_CHECK = ['.txt', '.c', '.cpp', '.h']


def check_line_endings(fname):
    '''
    true is good, false is bad
    '''
    with open(fname) as fdesc:
        for line in fdesc:
            line = line.rstrip('\n')
            if len(line) > 0:
                if line[len(line) - 1] == ' ' or line[len(line) - 1] == '\t':
                    return False

    return True

bad_files = 0
for root, dirs, files in os.walk('.'):
    for name in files:
        fullname = join(root, name)
        for end in TYPES_TO_CHECK:
            if fullname.endswith(end):
                if not check_line_endings(fullname):
                    print(fullname + " contains trailing whitespace")
                    bad_files += 1

exit(0 if bad_files == 0 else 1)
