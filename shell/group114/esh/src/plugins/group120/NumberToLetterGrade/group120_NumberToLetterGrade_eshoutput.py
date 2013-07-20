assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("Congratulations, you got an A") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"
