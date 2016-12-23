#!/usr/bin/env python3

import argparse
import os.path
import re

def full_identifier(filename):
    parts = os.path.normpath(filename).split(os.sep)
    parts = [to_identifier(part) for part in parts]
    print(filename, parts)
    return parts

def to_identifier(name):
    ident = re.sub(r'[^0-9a-zA-Z]', '_', name)
    if ident[0].isnumeric():
        return "_"+ident
    return ident

resource_impl_template = """
static const unsigned char {data_identifier}[] = {{
    {resource_content}
}};

namespace RESOURCE {{
{namespaces_open}
const std::string {identifier} = init_resource("{env_identifier}", {data_identifier}, {resource_len});
{namespaces_close}
}}
"""

resource_getter_template = """
    if (name == "{common_name}")
        return RESOURCE::{identifier};
"""

resource_decl_template = """{namespaces_open}
extern const std::string {identifier};
{namespaces_close}"""

class Resource:
    def __init__(self, base_dir, filename):
        filename = filename.strip()
        self.filename = filename
        self.identifier = full_identifier(filename)
        with open(os.path.join(base_dir, filename), 'rb') as f:
            self.data = f.read()

    def dump_impl(self):
        nb_row = len(self.data)//16 + (1 if len(self.data) % 16 else 0)
        sliced = (self.data[i*16:(i+1)*16] for i in range(nb_row))

        return resource_impl_template.format(
            data_identifier="_".join([""]+self.identifier),
            resource_content=",\n    ".join(", ".join("{:#04x}".format(i) for i in r) for r in sliced),
            resource_len=len(self.data),
            namespaces_open=" ".join("namespace {} {{".format(id) for id in self.identifier[:-1]), 
            namespaces_close=" ".join(["}"]*(len(self.identifier)-1)),
            identifier=self.identifier[-1],
            env_identifier="RES_"+"_".join(self.identifier)+"_PATH"
        )
    
    def dump_getter(self):
        return resource_getter_template.format(
            common_name=self.filename,
            identifier="::".join(self.identifier)
        )

    def dump_decl(self):
        return resource_decl_template.format(
            namespaces_open=" ".join("namespace {} {{".format(id) for id in self.identifier[:-1]), 
            namespaces_close=" ".join(["}"]*(len(self.identifier)-1)),
            identifier=self.identifier[-1]
        )
    


master_c_template = """//This file is automaically generated. Do not modify it.

#include <stdlib.h>
#include <fstream>
#include <exception>
#include "{basename}"

class ResourceNotFound : public std::runtime_error {{
  public:
    ResourceNotFound(const std::string& what_arg):
      std::runtime_error(what_arg)
    {{ }};
}};

static std::string init_resource(const char* name, const unsigned char* content, int len)
{{
    char * resPath = getenv(name);
    if (NULL == resPath)
        return std::string(reinterpret_cast<const char*>(content), len);
    
    std::ifstream ifs(resPath);
    if (!ifs.good())
        return std::string(reinterpret_cast<const char*>(content), len);
    return std::string( (std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()   ));
}}

const std::string& getResource(const std::string& name) {{
{RESOURCES_GETTER}
    throw ResourceNotFound("Resource not found.");
}}

{RESOURCES}

"""

def gen_c_file(resources, basename):
    return master_c_template.format(
       RESOURCES="\n\n".join(r.dump_impl() for r in resources),
       RESOURCES_GETTER="\n\n".join(r.dump_getter() for r in resources),
       basename=basename
    )
 


master_h_template = """//This file is automaically generated. Do not modify it.
#ifndef KIWIX_{BASENAME}
#define KIWIX_{BASENAME}

#include <string>

namespace RESOURCE {{
    {RESOURCES}
}};

const std::string& getResource(const std::string& name);

#endif // KIWIX_{BASENAME}

"""

def gen_h_file(resources, basename):
    return master_h_template.format(
       RESOURCES="\n    ".join(r.dump_decl() for r in resources),
       BASENAME=basename.upper()
    )

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxxfile',
                        help='The Cpp file name to generate')
    parser.add_argument('--hfile',
                        help='The h file name to generate')
    parser.add_argument('resource_file',
                        help='The list of resources to compile.')
    args = parser.parse_args()

    base_dir = os.path.dirname(os.path.realpath(args.resource_file))
    with open(args.resource_file, 'r') as f:
        resources = [Resource(base_dir, filename) for filename in f.readlines()]

    h_identifier = to_identifier(os.path.basename(args.hfile))
    with open(args.hfile, 'w') as f:
        f.write(gen_h_file(resources, h_identifier))

    with open(args.cxxfile, 'w') as f:
        f.write(gen_c_file(resources, os.path.basename(args.hfile)))

