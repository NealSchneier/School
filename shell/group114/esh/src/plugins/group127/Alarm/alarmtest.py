#!/usr/bin/python
# @author marknach+panta
# Quadratic Test: Start esh, and test alarm command with a 0 wait time
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
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile, args=['-p','/web/courses/cs3214/fall2011/projects/student-plugins/group127/Alarm'])
atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2

c.sendline("alarm 0");
assert c.expect("BEEP BEEP TIME TO WAKE UP") == 0, "Output printed was incorrect"


shellio.success()
