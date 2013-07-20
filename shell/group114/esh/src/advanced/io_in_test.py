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

#Removes whats in the filea
c.sendline("clear");
c.sendline("rm test_io_in.txt");
#Starting the file input 
c.sendline("echo hello >> test_io_in.txt");
c.sendline("ls >> test_io_in.txt");
time.sleep(0.5);
c.sendline("cat < test_io_in.txt");

#check for the two strings input
assert c.expect_exact("hello") == 0, "hello is there";
assert c.expect_exact("test_io_in.txt") == 0, "bye is there";

#Check some other input
c.sendline("echo adios >> test_io_in.txt");
c.sendline("cat < tesst_io_in.txt");
#check for the last input inserted
assert c.expect_exact("adios") == 0, "adios is there";

shellio.success()
