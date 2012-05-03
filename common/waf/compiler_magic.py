#! /usr/bin/env python
"""
Experimental: specify --use-compiler-environment-magic at configure time to use

1. Attempt to figure out what LD_LIBRARY_PATHs are needed to run the compiler
2. Attempt to figure out what RPATH is needed to run programs compiled with 
   said compiler
   
This is necessary for nonstandard compiler installations where the compiler
is brought into the environment by setting LD_LIBRARY_PATH.

This information is used so that after configuration, ./waf doesn't need to have
the original environment setup to correctly run the compiler, and furthermore
that compiled binaries don't need to have the environment setup to execute.
"""

import waflib
from waflib import Task
from waflib.TaskGen import after_method, feature

def get_runtime_paths(self, what):
    """
    Determine what components of LD_LIBRARY_PATH are necessary to run `what`
    """
    from os import environ
    from os.path import dirname
        
    self.env.stash()
    self.env.LD_LIBRARY_PATH = environ.get("LD_LIBRARY_PATH", "")
    
    try:
        # Run ldd
        out, err = self.bld.cmd_and_log([self.env.LDD, what], 
                                        output=waflib.Context.BOTH)
    finally:
        self.env.revert()
    
    # Parse ldd output to determine what paths are used by the dynamic linker
    maybe_paths = set()
    for line in out.split("\n"):
        parts = line.split()
        if not parts: continue
        if parts[1] == "=>": maybe_paths.add(dirname(parts[2]))
        
    return maybe_paths & set(environ.get("LD_LIBRARY_PATH", "").split(":"))

class check_binary_paths(Task.Task):
    def run(self):
        """
        Discover what RPATHs are necessary for a binary compiled with our
        compiler
        """
        conf_ctx = self.generator
        
        filename = self.inputs[0].abspath()
        compiled_prog_paths = get_runtime_paths(conf_ctx, filename)
        
        conf_ctx.env.append_value("RPATH", list(compiled_prog_paths))

@feature('check_compiler_paths')
@after_method('apply_link')
def check_compiler_paths(self):
    """
    Attempt to discover what LD_LIBRARY_PATHs are necessary to run cc1
    """
    self.create_task('check_binary_paths', self.link_task.outputs[0])

    out, err = self.bld.cmd_and_log(self.env.CXX + ["-print-prog-name=cc1"],
                                    output=waflib.Context.BOTH)
    
    compiler_paths = get_runtime_paths(self, out.strip())
    self.env.append_value("LD_LIBRARY_PATH", list(compiler_paths))

@feature('c', 'cxx')
@after_method('apply_link')
def setup_compiler_env(self):
    """
    Inject paths into LD_LIBRARY_PATH before running the compiler
    """
    from os import environ
    ld_library_path = self.env["LD_LIBRARY_PATH"]
    if "LD_LIBRARY_PATH" in environ:
        ld_library_path += environ["LD_LIBRARY_PATH"].split(":")
    if ld_library_path:
        this_env = environ.copy()
        this_env["LD_LIBRARY_PATH"] = ":".join(ld_library_path)
        self.env.env = this_env
        
def options(ctx):
    ctx.load('compiler_c compiler_cxx')
    
    ctx.add_option('--use-compiler-environment-magic', action="store_true",
        help="Attempt to automatically determine what paths are required to "
             "make a working compiler environment")

def configure(ctx):
    ctx.load('compiler_c compiler_cxx')
    
    if ctx.options.use_compiler_environment_magic:
        ctx.find_program("ldd", var="LDD")
        
        # Attempt to determine what runtime paths are needed to use the compiler
        x = ctx.run_c_code(features="cxx cxxprogram check_compiler_paths", 
                           env=ctx.env, compile_filename="test.cxx", code="""
            #include <iostream>
            using namespace std;
            int main(int argc, char* argv[]) { 
                cout << "Hello world" << endl; return 0; 
            }
        """)
