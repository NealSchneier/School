assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("300.00 K") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("27.00 C") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("-40.00 F") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("-40.00 C") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("422.00 K") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect("-423.67 F") == 0, "Incorrect Output"
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

assert c.expect_exact("exit\r\n") == 0, "Shell output extraneous characters"
