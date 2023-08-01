# LewiScript
The hardest part about making a scripting language is coming up with a clever name and desperately trying not to name it a single letter. Yes the name is still up for change.

# Why
I was wondering if it would be hard to make a scripting language that natively integrates C++. Of course i could use Lua or Python's ``ctypes`` but i wanted to try my own hand at it. And boy has that hand been burned a lot already.

# Native types
```
# there are currently only 4 native types
# there are plans for more: proper bool type, maps, etc
var number = 5 
var string = "number = " + String(number) 
var lambda = fn(n): n * 2 end
var array = [number, string, lambda]
var range = Range(0, 2, 1)
var iterator = Iterator(range)
var member_function = array.size
import "some.dll" as module
```

# Native C++ support (somewhat)
```
# Dll's can be loaded in using the import keyword but are required to be aliased
import "LibIO.dll" as io

# Functions within the dll can be accessed with a member expression
# Internally it calls GetProcAddress and will throw if the function was not found
# Make sure to use 'extern "C"' to avoid name mangling
io.print("Print: ", 5)
```
In C++ the ``print`` function is implemented as follows:
```cpp
extern "C" auto print(std::span<LeObject> args, MemoryManager& mem) -> LeObject
```

# Loops
The language currently supports for and while loops, where for loops require a custom iterator type.
```
for i in Range(0, 10, 2):
  print(i)
end

var iter = Iterator(Range(0, 10, 2))
while iter:
  print(iter.next())
end
```

# Functions
```
# inherently all functions are also lambda's
# This declaration places the function func within the global namespace
fn func():
  return 5
end
# This one does not, it assigns a lambda to a variable
var func = fn(): return 5 end

# Functions can be declared within functions, however those will not be declared in the global namespace so besides syntax there is no difference between it and a lambda
fn func():
  fn func2(): return 5 end
  return func2
end
```

# Builtin functions
```
type      # Implements Type->type_name()
print     # Prints arguments through to cout
String    # Implements Type->make_string() can also be seen as string constructor
Iterator  # Implements Type->iterator()
Range     # Range type constructor

# Fun fact: u can use them all at once in a single expression
print(String(type(Iterator(Range(1)))))
```

# Line endings
While languages like Python parse expressions till a new line occurs. Here i wanted to experiment with a lack of line endings. 
```
# No endings means this is valid
var a = 5 var b = 6 var c = 7

# This does mean issues can occur, here for example the unary '-' operator will be parsed as a binary operation between 5 and x
fn func(x):
  x = x + 5
  -x
end

# This can be fixed very simply by using an explicit return
fn func(x):
  x = x + 5
  return -x
end
```
