import subprocess
import numpy as np
import struct

notes = 1000
rates = 5
params = 10

stream = struct.pack('i', notes) + struct.pack('i', rates) + struct.pack('i', params)
proc = subprocess.run(['compute.exe'], check=True, capture_output=True, input = stream)


def read_output():
    struct_format = "5is"
    struct_size = struct.calcsize(struct_format)

    custom_structs = []
    for i in range(0, len(proc.stdout), struct_size):
        struct_data = proc.stdout[i:i+struct_size]
        custom_struct = struct.unpack(struct_format, struct_data)
        custom_structs.append(custom_struct)

    # Print the custom struct objects
    for cs in custom_structs:
        print(cs)