# LewiScript
The hardest part about making a scripting language is coming up with a clever name and desperately trying not to name it a single letter. Yes the name is still up for change.

# Why
I was wondering if it would be hard to make a scripting language that natively integrates C++. This because i like the simplicity of languages like Python or GdScript in Godot but often find myself annoyed when i want to write something that requires more speed but can't very easily write it in C++ without jumping through a lot of hoops.

# Syntax

```
# functions will return the top object on the stack of the function
# an empty return can be added to make the function return Null or to break the function quickly
fn add(a, b):
  a + b
end # blocks have to be closed with the end keyword

fn clamp(n, l, h):
  if n > h:
    return h
  elif n < l:
    return l
  end 
  return n
end

var number = 5
var string = "Hello!"
var lambda = fn(n): n * 2 end
var array = [number, string, lambda]

while number > 0:
  number = number - 1
end
```
