import subprocess
import time
import sys
import socket

def get_version_info():

    try:
        git_revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode("utf-8") .split("\n")[0]
        git_branch = subprocess.check_output(["git", "rev-parse","--abbrev-ref", "HEAD"]).decode("utf-8").split("\n")[0]
    except (subprocess.CalledProcessError, OSError):
        git_revision = ""
        git_branch = "non-git"

    def read_version():
        with open("VERSION") as f:
            return f.readline().strip()

    build_datetime = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime())
    version_number = read_version()

    hostname = socket.gethostname()

    return git_revision, git_branch, build_datetime, version_number, hostname

def print_version_number():
    sys.stdout.write(get_version_info()[3])

if __name__ =="__main__":

    output_file = sys.argv[1]
    with open(output_file, "w") as fout:
        fout.write("""#pragma once 

namespace knx{{
namespace version{{

auto constexpr git_revision = u8"{0}";
auto constexpr git_branch = u8"{1}";
auto constexpr build_datetime = u8"{2}";
auto constexpr version_number = u8"{3}";
auto constexpr build_hostname = u8"{4}";


}}
}}
        
""".format(*get_version_info()))

