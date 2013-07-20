#!/usr/bin/python
#
# Tests advanced IO_Out
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

#This is sending the command to the command line.
c.sendline("echo hello > test_io_out.txt");

#This checks what's in the test.txt file by pulling up the test.
c.sendline("cat test_io_out.txt");
time.sleep(0.5);
assert c.expect_exact("hello") == 0, "The echo was displaced"

#Another check for the hello
c.sendline("ls > test_io_out.txt");
assert c.expect_exact("test_io_out.txt") == 0, "The test file is in the directory"

#Utilizing another command with echo
c.sendline("echo abc123!@# > test_io_out.txt");
assert c.expect_exact("test_io_out.txt") == 0, "Testing with another echo"

shellio.success()
