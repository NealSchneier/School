import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

def force_shell_termination(shell_process): c.close(force=True)

definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'): logfile = def_module.logfile

c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile, args=['-p', 'plugins/'])
atexit.register(force_shell_termination, shell_process=c)

c.timeout = 2

assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("RGBToHEX 76 23 190")
assert c.expect("Equivalent hex value: 4C17BE") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

c.sendline("exit");
assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"

shellio.success()
