#!/usr/bin/python
#
# Sample test script for plugins.
# Test for 'cd' plugin
#
import sys, imp
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check


#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile, args=['-p','.'])

c.timeout = 2

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("mkdir a")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"
c.sendline("touch a/b")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"
c.sendline("cd a")
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"
c.sendline("ls b")

assert c.expect("b") == 0, "cd failed"

shellio.success()
