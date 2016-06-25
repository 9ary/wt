#!/usr/bin/env python3

import argparse
import sys

import shogun

cflags_common = [ "-Weverything", "-Wno-padded", "-fdiagnostics-color=always" ]
cflags_common += [ "-O2", "-flto" ]
cflags_common += [ "-pipe" ]
cflags_common += [ "-D_XOPEN_SOURCE=700", "-D_DEFAULT_SOURCE" ]
cflags_common += [ "-I." ]

cflags = " ".join(cflags_common + [ "-std=c11" ])

parser = argparse.ArgumentParser(description = "Configure the build")
parser.add_argument("--cflags", action = "store_true", help = "Print cflags")
args = parser.parse_args()

if args.cflags:
    print(cflags)
    sys.exit()

obj = shogun.Objects("src/*.c", "cc", "o")
libwt = shogun.Objects("libwt/*.c", "cc", "o")
exe = shogun.Assembly("$builddir/wt", "ccld", obj, libwt,
        options = { "libs": "" })
comp_flags = shogun.Variables(cflags = cflags)

shogun.build(obj, libwt, exe, comp_flags)
