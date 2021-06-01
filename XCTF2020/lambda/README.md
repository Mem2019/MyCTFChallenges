## Lambda Calculus

See [this](http://www.cs.columbia.edu/~sedwards/classes/2012/w4115-fall/lambda.pdf) for detail.

The program implements a lambda calculus evaluator in C. The lambda calculus is expressed using a C structure containing an union. The symbol is represented using a `uint8` number. The representation uses pointer to another lambda calculus structure to express nested structure (e.g. body part of lambda function). When creating a lambda calculus expression, we usually create a C array, and use `array + offset` to represent nested structure. For example, `true` of church boolean is:

```c
Exp churchTrue[] =
    // symbol is uint8, nested structure pointer is obtained using arr+off 
	{{.type = kLambda, .u = {.lambda = {.arg = 1, .body = churchTrue + 1}}},
	{.type = kLambda, .u = {.lambda = {.arg = 0, .body = churchTrue + 2}}},
	{.type = kSymbol, .u = {.symbol = 1}}};
// λx.λy.x
```

The return type of `val` is a closure, if we want to know if the return value is `true` or `false` in church boolean representation, we can use the `churchBool` function.

`compiler.py` is a script used to generate the lambda expression for the challenge. First of all I implement church boolean `true`, `false`, `and`, `or` and `not`, then I use these expressions to implement `xor`. These are church boolean primitive that we will use later. Note that, when generation symbol, I choose random number instead of fixed number to examine players' ability to identify alpha equivalence. 

Then I implements a small compiler to transform lambda calculus expression, which is in Python array form, to C array form compatible with my C lambda calculus interpreter above.

We also need an array of lambda expression to receive input. One input is used to represent one bit of flag, since flag length is `0x30`, the number of input lambda expressions is `0x30 * 8`. There are 2 types of input, `λ1.λ0.?` and `λ0.λ1.?`. For the first one, the value of church boolean is same as value of `?`; for the second one, the value of church boolean is inverse to value of `?`. These 2 types of inputs are generated randomly, and the status is recorded in array `invSeq`. Therefore, when putting the flag into the lambda expression, we need to consider its type. This is also used to examine alpha equivalence knowledge of players.

Now we come to the logic of lambda expression of this challenge. The lambda expression is not hard: it firstly `xor` input with given church boolean generated according to flag(e.i. it should be inverse to value of input lambda expression with flag as content), then `and` all of the results altogether. If the final result is `true`, the flag is correct. 

The lambda expression is like following:

```commonlisp
(and (xor flag0 given0) (xor flag1 given1) ...)
```

## Reverse Engineering

The C code is not hard to reverse, I even putted some hints in assertion to make the challenge easier. The only possible hard part here is to identify this is a lambda calculus evaluator used to evaluate church boolean, and this is not too hard either given the challenge name is `lambda` and a `churchBool` string in assertion message. We can thus figure out C structure of lambda expression.

The hardest part is to recover and understand the huge lambda expression from C array. The idea is to match different lambda expression patterns of church boolean and replace lambda expressions to more meaningful strings. `dump.py` is used to dump lambda expression from IDA, and `solve.py` is used to solve the flag.