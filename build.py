#!/usr/bin/env python3

import subprocess
import threading
import sys

def run_command(command_args, description):
    """
    Runs a command in a subprocess, capturing its output. 
    Raises an exception if the command fails.
    """
    print(f"Starting: {description}")
    process = subprocess.Popen(
        command_args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )

    # Continuously read output and print to console
    for line in process.stdout:
        print(f"[{description}] {line}", end="")

    process.wait()
    if process.returncode != 0:
        raise RuntimeError(f"'{description}' failed with exit code {process.returncode}")
    print(f"Finished: {description}\n")

def main():
    # 1) Conan install for Debug
    conan_debug_cmd = ["conan", "install", ".", "--build=missing", "-s", "build_type=Debug"]
    # 2) Conan install for Release
    conan_release_cmd = ["conan", "install", ".", "--build=missing", "-s", "build_type=Release"]

    # Create threads for parallel execution of the conan install commands
    debug_thread = threading.Thread(target=run_command, args=(conan_debug_cmd, "Conan Debug"))
    release_thread = threading.Thread(target=run_command, args=(conan_release_cmd, "Conan Release"))

    # Start both
    debug_thread.start()
    release_thread.start()

    # Wait for both to finish
    debug_thread.join()
    release_thread.join()

    # 3) CMake build
    cmake_cmd = ["cmake", "--preset", "conan-default", "-DDEV_MACHINE=ON"]
    run_command(cmake_cmd, "CMake Configure & Build")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"ERROR: {str(e)}", file=sys.stderr)
        sys.exit(1)
