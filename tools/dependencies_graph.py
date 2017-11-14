#!/usr/bin/python3

from pprint import pprint

import os
import re
import pathlib
import argparse
import tempfile
import subprocess

def parse_dependencies(files, exclude):

    file_to_deps = {}

    include_regex = re.compile("^.*\#include\s+\"(.+)\"$")

    for f in files:
        name = os.path.basename(f.name)
        deps = []
        file_to_deps[name] = deps

        for line in f.readlines():
            match = include_regex.match(line)
            if match:
                dep = match.group(1)
                if dep not in exclude:
                    deps.append(dep)

    return file_to_deps

def gen_dot(deps, f):

    print("digraph chip8_dependencies {", file=f)

    for path in deps:
        for dep in deps[path]:
            print('    "' + path + '" -> "' + dep + '";', file=f)
    print("}", file=f)

def gen_graph(deps, output):
    with tempfile.NamedTemporaryFile(mode="w") as tmp_dot_file:
        gen_dot(deps, tmp_dot_file)
        tmp_dot_file.flush()
        subprocess.check_call(['dot','-Tpdf', tmp_dot_file.name, "-o", output])

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputs", type=argparse.FileType('r'), required=True, nargs="+")
    parser.add_argument("--output", type=argparse.FileType('w'), required=True)
    parser.add_argument("--exclude", type=str, nargs="+", default=[])
    args = parser.parse_args()

    deps = parse_dependencies(args.inputs, args.exclude)

    gen_graph(deps, args.output.name)

