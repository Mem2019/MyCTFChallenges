# Description

[Last time](https://github.com/Mem2019/MyCTFChallenges/tree/master/XCTF2020/lambda) I made a lambda reverse challenge. This time the revenge comes back.

Read [this](http://www.cs.cmu.edu/~venkatg/teaching/15252-sp20/notes/lambda-calculus-slides.pdf) for more detail.

# Solution

## Challenge

This challenge is a revenge of [lambda](https://github.com/Mem2019/MyCTFChallenges/tree/master/XCTF2020/lambda) in XCTF Final 2020. Since its algorithm is too simple, some teams solved it unintendedly by guessing. Therefore, this year, we have a much more complicated revenge. It is basically a matrix multiplication via recursion using Y Combinator.

To mitigate the increased difficulty of algorithm in lambda expression, we reduce the effort of reverse engineering the binary itself. To be specific, source code of the challenge is provided to players in file `main.c`. The file is a lambda calculus interpreter similar to the previous [one](https://github.com/Mem2019/MyCTFChallenges/blob/master/XCTF2020/lambda/main.c). However, this time the reference counter is implemented to free unused memory. In addition, in order to support Y Combinator, the interpreter is changed to call-by-need lazy evaluation. In other word, a lambda expression is not evaluated when it is passed as an argument, but is only evaluated when it has to be resolved.

The algorithm of the challenge can be written as pseudo code as follows.

```python
# To prevent people from guessing the algorithm,
# we slightly pre-process and post-process these values
def chall_dot(a, b):
	assert len(a) == len(b) and len(a) == 3
	a += [0, 1, 3]
	b -= [18, 15, 13]
	ret = 7
	for i in range(0, 3):
		ret += a[i] * b[i]
	return ret

matrix = [...] # a 3x3 matrix
res = [...] # a 3-element vector
x = input() # a 3-element vector, taken from input

for i in range(0, 3):
	if chall_dot(matrix[i], x) != res[i]:
		print("Wrong")
		exit(1)

print("Correct")
```

The algorithm is essentially a matrix multiplication, but the dot product is customized in order to prevent players from guessing it without reversing. File `chall.lisp` is the Lisp version of the pseudo code.

The length of flag is 33, and is separated to 11 parts, each of which contains 3 bytes. Each part has a separate program to identify whether the input is correct, each of which has different `matrix` and `res` . The numbers and lists are represented in Church encoding, and related operations are also implemented. More details can be found in file `compiler.py`. In order to have recursion supported, there is also Y Combinator.

## Reverse Engineering

Since we already have `main.c`, we only need to reverse the lambda expression. The intended approach is to implement a decompiler that matches each Church encoding using alpha equivalence. Using such decompiled Lisp program, we can figure out the algorithm and thus the required input vector. The details can be found in file `decompiler.py`.
