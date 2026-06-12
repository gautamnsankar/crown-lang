# CrownLang
A simple, compiled, strongly, and statically typed Turing-complete progrmaming language.

### Basic Printing Example
```
import std;

fn main() -> void {
    std.print("Hello World.");
}
```

### Math Example
```
import math;
import std;

fn main() -> void {
    let x: double = math.pi * 2;
    std.print(math.sin(x));
}
```