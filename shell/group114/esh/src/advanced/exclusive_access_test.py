#!/usr/bin/python
#
# Block header comment
#
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
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile)

atexit.register(force_shell_termination, shell_process=c)

#Setups what needs to be done to check for terminal control
c.sendline("pico exclusive_access.txt");

#Header of pico has UW PICO 5.05
assert c.expect_exact("UW PICO 5.05") == 0, "Command didn't go through"

c.sendcontrol('x')
#Check if terminal is given to shell again.
c.sendline("echo hello")
assert c.expect_exact("hello") == 0, "Shell has access again"


shellio.success()
