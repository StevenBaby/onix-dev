import os
import binascii

DIRNAME = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.abspath(os.path.join(DIRNAME, "../../build"))
SYSTEMBIN = os.path.join(BUILD, 'system.bin')
OFFSET = 0x8000


with open(SYSTEMBIN, 'rb') as file:
    data = file.read()
    chksum = binascii.crc32(data[OFFSET:])

print(f"kernel size: {len(data) - OFFSET:#x}")
print(f"kernel chksum: {chksum:#010x}")
