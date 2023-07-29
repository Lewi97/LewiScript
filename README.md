# LewiScript
The hardest part about making a scripting language is coming up with a clever name and desperately trying not to name it a single letter. Yes the name is still up for change.

# Why
I was wondering if it would be hard to make a scripting language that natively integrates C++. Of course i could use Lua or Python's ``ctypes`` but i wanted to try my own hand at it. And boy has that hand been burned a lot already.

# Native types

```
# there are currently only 4 native types
# there are plans for more: modules, proper bool type, maps, etc
var number = 5 
var string = "Hello!" 
var lambda = fn(n): n * 2 end
var array = [number, string, lambda]
```

# Functions
```
# inherently all functions are also lambda's
fn func():
  return 5
end
# is 'almost' equal to
var func = fn(): return 5 end
# the 'fn' keyword places the 'func' identifier in the global namespace so it can be reached by other functions

# functions can be declared within functions, however those will not be declared in the global namespace
fn func():
  fn func2(): return 5 end
  return func2
end
```
