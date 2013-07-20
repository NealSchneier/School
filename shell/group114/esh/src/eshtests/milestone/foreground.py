#!/usr/bin/python
#
# Tests that esh can run commands in the foreground
# 

import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check


#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)

# start shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=None)
atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 4 seconds
c.timeout = 4

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# run a command
c.sendline("/usr/bin/gcc")

assert c.expect("gcc: no input files") == 0, "Shell did not start gcc"

# make sure shell returns to prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# send EOF
c.sendeof()

# send SIGINT in case the EOF doesn't quit their shell
c.sendintr()

shellio.success()
