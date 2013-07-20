#!/usr/bin/python
#
# fg_test: tests the fg command
# 
# Test the fg command for bringing a command back to the foreground.
# Requires the following commands to be implemented
# or otherwise usable:
#
#	fg, sleep, ctrl-c control, ctrl-z control
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

# spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile)
atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (1)"



# run a command
c.sendline("sleep 30 &")

#snag the jobid and pid of the sleep command
(jobid, pid) = shellio.parse_regular_expression(c, def_module.bgjob_regex)

#check the prompt prints
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (2)"



#resume the sleep command
c.sendline(def_module.builtin_commands['fg'] % jobid)

#wait until it takes over the foreground process group
# This is NOT a bullet-proof fix, you may fail on occasion!
time.sleep(1)

#send the command back to the background
c.sendcontrol('z')

#check the prompt prints
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (3)"



#run a command to the background
c.sendline("sleep 300 &")

#snag the jobid and pid of the second sleep command
(jobid2, pid2) = shellio.parse_regular_expression(c, def_module.bgjob_regex)

#check the prompt prints
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (4)"

#resume the command by its jobid
c.sendline(def_module.builtin_commands['fg'] % jobid)

#wait until it takes over the foreground process group
# This is NOT a bullet-proof fix, you may fail on occasion!
time.sleep(1)

#Ensure that the sleep is in the foreground process group via /proc/
#assert proc_check.check_pid_fgpgrp(pid),  "Error, the pid's process group is \
#                                           not the foreground process group"

#send the command back to the background
c.sendcontrol('z')

#check the prompt prints
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (5)"



#resume the command by its jobid
c.sendline(def_module.builtin_commands['fg'] % jobid2)

#wait until it takes over the foreground process group
# This is NOT a bullet-proof fix, you may fail on occasion!
time.sleep(1)

#Ensure that the sleep is in the foreground process group via /proc/
#assert proc_check.check_pid_fgpgrp(pid2),  "Error, the pid's process group is \
#                                           not the foreground process group"

#end the process
c.sendintr()

#check that the prompt prints
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt (6)"



#resume the first sleep command
c.sendline(def_module.builtin_commands['fg'] % jobid)

#wait until the process takes over the foreground process group
# This is NOT a bullet-proof fix, you may fail on occasion!
time.sleep(1)


#Ensure that the sleep is in the foreground process group via /proc/
#assert proc_check.check_pid_fgpgrp(pid),  "Error, the pid's process group is \
#                                           not the foreground process group"



#exit
c.sendline("exit");
assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"


shellio.success()
