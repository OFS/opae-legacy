#!/usr/bin/env python
# Copyright(c) 2017, Intel Corporation
#
# Redistribution  and  use  in source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of  source code  must retain the  above copyright notice,
#  this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name  of Intel Corporation  nor the names of its contributors
#   may be used to  endorse or promote  products derived  from this  software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
# IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
# LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
# CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
# SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
# INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
# CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
import sys
import argparse
import re
import json
import zipfile
import uuid
from afu import AFU

AFU_JSON_MGR_EXEC = "afu_json_mgr"
DESCRIPTION = 'Intel FPGA AFU JSON Manager'

USAGE = """
{0}

{1} <cmd> [options]

The following values for <cmd> are currently supported:
\t help - displays this message
\t create-json - creates a minimal JSON file describing an AFU
\t json-info - extract information from JSON file for use in C or Verilog

{1} <cmd> --h will give command specific help
""".format(DESCRIPTION, AFU_JSON_MGR_EXEC)


# Create an AFU JSON file, filling in a few key fields.
def create_json(subargs):
    # Read the template JSON file
    filepath = os.path.dirname(os.path.realpath(__file__))
    template_path = "schema/afu_template.json"
    afu = AFU()
    if (zipfile.is_zipfile(filepath)):
        archive = zipfile.ZipFile(filepath, 'r')
        afu.load_afu_desc_file_hdl(archive.open(template_path, "r"))
    else:
        afu.load_afu_desc_file_hdl(open(os.path.join(filepath,
                                                     template_path), "r"))

    accel = afu.afu_json['afu-image']['accelerator-clusters'][0]
    accel['name'] = subargs.name

    # Top-level interface specified?
    if (subargs.top_ifc):
        afu.update_afu_json(['afu-image/afu-top-interface/class:' +
                             subargs.top_ifc])

    # Either set the specified UUID or pick one
    if (subargs.uuid):
        accel['accelerator-type-uuid'] = subargs.uuid
    else:
        accel['accelerator-type-uuid'] = str(uuid.uuid1())

    # The output file name is either the AFU name or a specified file path
    json_path = subargs.name + '.json'
    if (subargs.afu_json):
        json_path = subargs.afu_json
    print("Writing {0}".format(json_path))
    with open(json_path, 'w') as a:
        a.write(afu.dumps() + '\n')


def emit_header_comment(f, src):
    f.write('''//
// Generated by afu_json_mgr from {0}
//

'''.format(src))


# Flatten JSON into a simple, single level dictionary to emit as header files
def flatten_json(subargs):
    afu = AFU(subargs.afu_json)

    entries = dict()

    emit_types = (int, bool, str, str, float)
    # Don't emit some keys.  For example, user clock frequency may not be
    # known at compile time, so avoid emitting potentially false information.
    skip_keys = ['clock-frequency-low', 'clock-frequency-high']

    # Flattan all entries in afu-image that are of type emit_types
    image = afu.afu_json['afu-image']
    for k in sorted(image.keys()):
        if (isinstance(image[k], emit_types) and k not in skip_keys):
            tag = str(k).replace('-', '_')
            v = image[k]
            # Does it look like a number?
            if (not re.match('^[0-9.]+$', str(v))):
                v = '"' + str(v) + '"'
            entries['afu_image_' + tag] = v

    # Some special names, taken from other levels
    accel = image['accelerator-clusters'][0]
    entries['afu_accel_name'] = '"' + accel['name'] + '"'
    entries['afu_accel_uuid'] = accel['accelerator-type-uuid']
    try:
        # May not be present.  (Will become required eventually.)
        entries['afu_top_ifc'] = '"' + \
            afu.afu_json['afu-image']['afu-top-interface']['class'] + '"'
    except Exception:
        None

    return entries


# Emit C or Verilog header files based on an AFU JSON file
def json_info(entries, subargs):
    afu = AFU(subargs.afu_json)

    # C header
    if (subargs.c_hdr):
        print("Writing {0}".format(subargs.c_hdr))
        with open(subargs.c_hdr, 'w') as p:
            emit_header_comment(p, subargs.afu_json)
            p.write('#ifndef __AFU_JSON_INFO__\n')
            p.write('#define __AFU_JSON_INFO__\n\n')
            for k in sorted(entries.keys()):
                v = entries[k]
                if (k == 'afu_accel_uuid'):
                    v = '"' + v.upper() + '"'
                p.write('#define {0} {1}\n'.format(k.upper(), v))
            p.write('\n#endif // __AFU_JSON_INFO__\n')

    # Verilog header
    if (subargs.verilog_hdr):
        print("Writing {0}".format(subargs.verilog_hdr))
        with open(subargs.verilog_hdr, 'w') as p:
            emit_header_comment(p, subargs.afu_json)
            p.write('`ifndef __AFU_JSON_INFO__\n')
            p.write('`define __AFU_JSON_INFO__\n\n')
            for k in sorted(entries.keys()):
                v = entries[k]
                if (k == 'afu_accel_uuid'):
                    v = "128'h" + entries[k].replace('-', '_')
                p.write('`define {0} {1}\n'.format(k.upper(), v))
            p.write('\n`endif // __AFU_JSON_INFO__\n')


def run_afu_json_mgr():
    parser = argparse.ArgumentParser(usage=USAGE, add_help=False)
    parser.add_argument("cmd", nargs="?")
    parser.add_argument("remain_args", nargs=argparse.REMAINDER)
    args = parser.parse_args(sys.argv[1:])
    cmd_description = "{0} {1}".format(AFU_JSON_MGR_EXEC, args.cmd)
    subparser = argparse.ArgumentParser(description=cmd_description)
    subparser._optionals.title = 'Options'

    if args.cmd == "help" or not args.cmd:
        print(USAGE)

    elif args.cmd == "create-json":
        subparser.usage = "\n" + cmd_description + \
            " --name=<AFU_NAME> --top-ifc=<TOP_INTERFACE_CLASS>"\
            " --uuid=<AFU_UUID> --afu-json=<OUTPUT_JSON>\n"
        subparser.add_argument('--name', required=True,
                               help='AFU name (REQUIRED)')
        subparser.add_argument('--top-ifc', required=False,
                               default='ccip_std_afu',
                               help='Top-level interface class name. '
                               'Default is ccip_std_afu.  See the output of '
                               '"afu_platform_config --help" for a list of '
                               'top-level interface classes.')
        subparser.add_argument('--uuid', required=False,
                               help='Accelerator UUID (default: chosen at '
                               'random)')
        subparser.add_argument('--afu-json', required=False,
                               help='Output path for JSON file '
                               '(default <afu_name>.json)')
        subargs = subparser.parse_args(args.remain_args)
        create_json(subargs)

    elif args.cmd == "json-info":
        subparser.usage = "\n" + cmd_description + \
            " --afu-json=<INPUT_JSON>"\
            " --c-hdr=<OUTPUT_C_HDR_PATH>"\
            " --verilog-hdr=<OUTPUT_VERILOG_HDR_PATH>\n"
        subparser.add_argument('--afu-json', required=True,
                               help='Input path of JSON file. ')
        subparser.add_argument('--c-hdr', required=False,
                               help='Path of generated C header file. ')
        subparser.add_argument('--verilog-hdr', required=False,
                               help='Path of generated Verilog header file. ')
        subargs = subparser.parse_args(args.remain_args)
        entries = flatten_json(subargs)
        json_info(entries, subargs)

    else:
        raise Exception("{0} is not a command for {1}!".format(
            args.cmd, DESCRIPTION))


def main():
    try:
        sys.exit(run_afu_json_mgr())
    except Exception as e:
        print("ERROR: {0}".format(e.__str__()))
        sys.exit(1)


if __name__ == '__main__':
    main()
