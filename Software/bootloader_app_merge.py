"""
Copyright 2021 The Things Industries B.V.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from os import path
import hashlib, zlib, struct
import time
import sys
from intelhex import IntelHex

# define defaults to go into the metadata header
SIZEOF_SHA512     = int(512/8)
FIRMWARE_HEADER_VERSION = 1
header_format = ">1I2Q{}s1I".format(SIZEOF_SHA512)

if sys.version_info < (3,):
    def b(x):
        return bytearray(x)
else:
    def b(x):
        return x

def create_header(app_blob, firmwareVersion):
    # calculate the hash of the application
    firmwareHash = hashlib.sha256(app_blob).digest()

    # calculate the total size which is defined as the application size + metadata header
    firmwareSize = len(app_blob)

    print ('imageSize:    {}'.format(firmwareSize))
    print ('imageHash:    {}'.format(''.join(['{:0>2x}'.format(c) for c in b(firmwareHash)])))
    print ('imageversion: {}'.format(firmwareVersion))

    # construct struct for CRC calculation
    headerCRC = 0
    FirmwareHeader = struct.pack(header_format,
                                 FIRMWARE_HEADER_VERSION,
                                 firmwareVersion,
                                 firmwareSize,
                                 firmwareHash,
                                 headerCRC)

    # calculate checksum over header, including signatureSize but without headerCRC
    headerCRC = zlib.crc32(FirmwareHeader[:-4]) & 0xffffffff

    # Pack the data into a binary blob
    FirmwareHeader = struct.pack(header_format,
                                 FIRMWARE_HEADER_VERSION,
                                 firmwareVersion,
                                 firmwareSize,
                                 firmwareHash,
                                 headerCRC)

    return FirmwareHeader


def combine(bootloader_fn, app_fn, app_addr, hdr_addr, bootloader_addr, output_fn, version):
    ih = IntelHex()

    bootloader_format = bootloader_fn.split('.')[-1]

    # write the bootloader
    print("Using bootloader %s" % bootloader_fn)
    if bootloader_format == 'bin':
        print("Loading bootloader to address 0x%08x." % bootloader_addr)
        ih.loadbin(bootloader_fn, offset=bootloader_addr)
    else:
        print('Bootloader format can only be .bin or .hex')
        exit(-1)

    # write firmware header
    app_format=app_fn.split('.')[-1]
    with open(app_fn, 'rb') as fd:
        app_blob = fd.read()
    FirmwareHeader = create_header(app_blob, version)
    print("Writing header to address 0x%08x." % hdr_addr)
    ih.puts(hdr_addr, FirmwareHeader)

    # write the application
    if app_format == 'bin':
        print("Loading application to address 0x%08x." % app_addr)
        ih.loadbin(app_fn, offset=app_addr)
    elif app_format == 'hex':
        print("Loading application from hex file")
        ih.fromfile(app_fn, format=app_format)

    # output to file
    ih.tofile(output_fn, format=output_fn.split('.')[-1])

if __name__ == '__main__':
    from glob import glob
    import argparse

    parser = argparse.ArgumentParser(
        description='Combine bootloader with application adding metadata header.')

    def addr_arg(s):
        if not isinstance(s, int):
            s = eval(s)

        return s

    # specify arguments
    parser.add_argument('-b', '--bootloader',    type=argparse.FileType('rb'),     required=True,
                        help='path to the bootloader binary')
    parser.add_argument('-a', '--app',           type=argparse.FileType('rb'),     required=True,
                        help='path to application binary')
    parser.add_argument('-c', '--app-addr',      type=addr_arg,                    required=True,
                        help='address of the application')
    parser.add_argument('-d', '--header-addr',   type=addr_arg,                    required=True,
                        help='address of the firmware metadata header')
    parser.add_argument('-o', '--output',        type=argparse.FileType('wb'),     required=True,
                        help='output combined file path')
    parser.add_argument('-s', '--set-version',   type=int,                         required=True,
                        help='set version number', default=int(time.time()))

    # get and validate arguments
    parser._optionals.title = "arguments"
    args = parser.parse_args()

    bootloader_fn = path.abspath(args.bootloader.name)
    args.bootloader.close()
    app_fn = path.abspath(args.app.name)
    args.app.close()
    output_fn = path.abspath(args.output.name)
    args.output.close()
    app_addr = args.app_addr
    mem_start = 0
    header_addr = args.header_addr

    # combine application and bootloader adding metadata info
    combine(bootloader_fn, app_fn, app_addr, header_addr, mem_start,
            output_fn, args.set_version)

    # print the output file path
    print('Combined binary:' + output_fn)
