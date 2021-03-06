#!/usr/bin/env python
import sys
import os
from glob import glob

import overrides
from utils import memoize


@memoize
def get():
    aux_fs = overrides.get('CHPL_AUX_FILESYS', 'none')

    if aux_fs == 'hdfs':
        java_subdir = os.environ.get('JAVA_INSTALL', '')
        aux_fs_subdir = os.environ.get('HADOOP_INSTALL', '')

        # This will not check that all dependencies are satisfied..
        found_java = os.path.isdir(os.path.join(java_subdir, 'include'))
        found_hdfs = os.path.exists(os.path.join(aux_fs_subdir, 
                                                 'include', 'hdfs.h'))
        found_hdfs_lib = os.path.exists(os.path.join(aux_fs_subdir, 'lib',
                                                 'native', 'libhdfs.a'))
        if not found_java:
            sys.stderr.write("Warning: Can't find your Java installation\n")
        if not found_hdfs or not found_hdfs_lib:
            sys.stderr.write("Warning: Can't find your Hadoop installation\n")

    elif aux_fs == 'hdfs3':
        def fetchInfo(env, envtype, filename, err):
            directories = map(lambda z: z.lstrip(envtype), os.environ.get(env, '').split())
            res = sum([ os.path.exists(os.path.join(d, filename)) for d in directories ])

            if res < 1:
                sys.stderr.write(err)
                return False

            return True

        fetchInfo('CHPL_AUXIO_INCLUDE', '-I', 'hdfs.h', "Warning: Can't find your HDFS3 header file installation\n")
        fetchInfo('CHPL_AUXIO_LIBS', '-L', 'libhdfs3.a', "Warning: Can't find your HDFS3 static library installation\n")

    return aux_fs


def _main():
    aux_fs_val = get()
    sys.stdout.write("{0}\n".format(aux_fs_val))


if __name__ == '__main__':
    _main()
