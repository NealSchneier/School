#!/usr/bin/python
# @author marknach+panta
# Binary Converter
#

import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, proc_check, shellio, signal, time, threading

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

# spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile, args=['-p','plugins/'])
atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2

c.sendline("inttobin 4");
assert c.expect("The binary representation of 4 is 0100") == 0, "Output printed was incorrect"


c.sendline("inttobin 200");
assert c.expect("The binary representation of 200 is 11001000") == 0, "Output printed was incorrect"
shellio.success()
