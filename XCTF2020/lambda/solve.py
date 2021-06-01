import sys
import pwn

sys.setrecursionlimit(10000)

with open('lambda', 'rb') as fd:
	data = fd.read()

def get_qword(addr):
	global data
	addr -= 0x1000
	return pwn.u64(data[addr:addr+8])

def patch_qword(addr, val):
	global data
	addr -= 0x1000
	data = data[:addr] + pwn.p64(val) + data[addr+8:]

def flag_input(addr, count):
	for i in range(0, count):
		tmp = addr + 0x48 * i
		assert get_qword(tmp) == 137 or get_qword(tmp) == 0x10000 + i
		patch_qword(tmp, 0x10000 + i)

def parse_lambda(addr):
	t = get_qword(addr)
	if t == 0:
		return ["symbol", get_qword(addr + 8)]
	elif t == 1:
		return ["lambda", get_qword(addr + 8),
			parse_lambda(get_qword(addr + 16))]
	elif t == 2:
		return ["call", parse_lambda(get_qword(addr + 8)),
			parse_lambda(get_qword(addr + 16))]
	assert False

flag_input(0x12f470, 0x30 * 8)
chall = parse_lambda(0x12f450)

# convert (lambda (x) (lambda (y) ... body))
# to (lambda (x y ...) body)
def try_parse_lambda(expr):
	if expr[0] != 'lambda':
		return expr
	args = [expr[1]]
	expr = expr[2]
	while expr[0] == 'lambda':
		args.append(expr[1])
		expr = expr[2]
	return ['lambda', args, expr]

# convert (((func x) y) z ...)
# to (func x y z ...)
def try_parse_call(expr):
	if expr[0] != 'call':
		return expr
	params = [expr[2]]
	expr = expr[1]
	while expr[0] == 'call':
		params.append(expr[2])
		expr = expr[1]
	params.append(expr)
	return ['call', params[::-1]]

def try_parse(expr):
	expr = try_parse_call(try_parse_lambda(expr))
	if expr[0] == 'lambda':
		expr[2] = try_parse(expr[2])
	elif expr[0] == 'call':
		params = []
		for param in expr[1]:
			params.append(try_parse(param))
		expr[1] = params
	return expr

def churchXor(params, s0, s1):
	res = church(params[0])
	if res != 'or':
		return False

	# test and1
	if params[1][0] != 'call' or len(params[1][1]) != 3:
		return False
	args = params[1][1]
	if church(args[0]) != 'and':
		return False
	if args[1][0] != 'symbol' or args[1][1] != s0:
		return False
	if args[2][0] != 'call' or len(args[2][1]) != 2:
		return False
	if church(args[2][1][0]) != 'not':
		return False
	if args[2][1][1][0] != 'symbol' or args[2][1][1][1] != s1:
		return False

	# test and2
	if params[2][0] != 'call' or len(params[2][1]) != 3:
		return False
	args = params[2][1]
	if church(args[0]) != 'and':
		return False
	if args[1][0] != 'call' or len(args[1][1]) != 2:
		return False
	if church(args[1][1][0]) != 'not':
		return False
	if args[1][1][1][0] != 'symbol' or args[1][1][1][1] != s0:
		return False
	if args[2][0] != 'symbol' or args[2][1] != s1:
		return False

	return True







# convert church primitive to string
def church(expr):
	# expr should be try_parsed
	if expr[0] == 'symbol':
		assert expr[1] < 0x100
		return 'v%u' % expr[1]
	elif expr[0] == 'lambda' and len(expr[1]) == 2:
		p = expr[1][0]
		q = expr[1][1]
		if expr[2][0] == 'symbol':
			if expr[2][1] == expr[1][0]:
				return 'true'
			if expr[2][1] == expr[1][1]:
				return 'false'
			if expr[2][1] >= 0x10000:
				if expr[1][0] == 1 and expr[1][1] == 0:
					return 'i', (expr[2][1] - 0x10000)
				elif expr[1][0] == 0 and expr[1][1] == 1:
					return 'n', (expr[2][1] - 0x10000)
				else:
					assert False

		elif expr[2][0] == 'call' and len(expr[2][1]) == 3:
			params = expr[2][1]
			b = True
			for i in range(0, 3):
				b = b and params[i][0] == 'symbol'
			if b:
				if params[0][1] == p and params[1][1] == q and params[2][1] == p:
					return 'and'
				if params[0][1] == p and params[1][1] == p and params[2][1] == q:
					return 'or'
			if churchXor(params, q, p):
				return 'xor'


	elif expr[0] == 'lambda' and len(expr[1]) == 3:
		p = expr[1][0]
		a = expr[1][1]
		b = expr[1][2]
		if expr[2][0] == 'call' and len(expr[2][1]) == 3:
			params = expr[2][1]
			tmp = True
			for i in range(0, 3):
				tmp = tmp and params[i][0] == 'symbol'
			if tmp:
				if params[0][1] == p and params[1][1] == b and params[2][1] == a:
					return 'not'


# convert all church boolean primitives to readable strings
def church_all(expr):
	ret = church(expr)
	if not ret is None:
		return ret
	if expr[0] == 'lambda':
		expr[2] = church_all(expr[2])
	elif expr[0] == 'call':
		expr[1] = list(map(church_all, expr[1]))
	return expr

expr = church_all(try_parse(chall))
flag = [None] * (0x30 * 8)
while expr[0] == 'call' and expr[1][0] == 'and':
	tmp = expr[1][2]
	# print(tmp)
	assert tmp[0] == 'call' and tmp[1][0] == 'xor'
	b = tmp[1][2] != 'true'
	if tmp[1][1][0] == 'n':
		b = not b
	flag[tmp[1][1][1]] = b
	expr = expr[1][1]
assert expr == 'true'

# print(flag)
flag2 = [0] * 0x30
for i in range(0, 0x30):
	for j in range(0, 8):
		if flag[i * 8 + j]:
			flag2[i] |= 1 << j

print("".join(map(chr, flag2)))