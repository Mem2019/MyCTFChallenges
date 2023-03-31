import pwn
from compiler import *	

class Expr:
	def __init__(self, arr, idx):
		self.arr = arr
		self.idx = idx

def loadLambda(vaddr, off, size):
	def toIdx(val):
		if val & (~0xff) == 0xDEFC0719600:
			return -(val & 0xff)-1
		ret = val - vaddr
		assert ret % 0x18 == 0 and ret >= 0
		return ret // 0x18
	def procSymbol(data):
		return ("symbol", pwn.u64(data[8:16]))
	def procLambda(data):
		return ("lambda", pwn.u64(data[8:16]), toIdx(pwn.u64(data[16:24])))
	def procCall(data):
		return ("call", toIdx(pwn.u64(data[8:16])), toIdx(pwn.u64(data[16:24])))
	enum = [procSymbol, procLambda, procCall]
	with open("./lambda", 'rb') as fd:
		fd.seek(off)
		data = fd.read(size * 0x18)
	ret = []
	for i in range(0, size * 0x18, 0x18):
		ret.append(enum[pwn.u64(data[i:i+8])](data[i:i+0x18]))
	return Expr(ret, len(ret) - 1)

def updateChurches():
	for k in churches:
		res = []
		idx = compileLambda(churches[k](), res, None, 0xDEFC0719600)
		churches[k] = Expr(res, idx)

def findLastEqv(s, env, loc):
	for x in reversed(env):
		if x[loc] == s:
			return x[loc ^ 1] # hacky way to invert bit
	assert False

def alphaEqv(e0, e1, env):
	t0, t1 = e0.arr[e0.idx], e1.arr[e1.idx]
	if t0[0] != t1[0]:
		return False
	t = t0[0]
	if t == "call":
		return alphaEqv(Expr(e0.arr, t0[1]), Expr(e1.arr, t1[1]), env) and \
			alphaEqv(Expr(e0.arr, t0[2]), Expr(e1.arr, t1[2]), env)
	elif t == "lambda":
		env.append((t0[1], t1[1]))
		ret = alphaEqv(Expr(e0.arr, t0[2]), Expr(e1.arr, t1[2]), env)
		env.pop()
		return ret
	else:
		assert t == "symbol"
		return findLastEqv(t0[1], env, 0) == t1[1] and \
			findLastEqv(t1[1], env, 1) == t0[1]

def decodeNum(expr):
	arr = expr.arr
	if arr[expr.idx][0] != "lambda":
		return None
	f = arr[expr.idx][1]
	idx = arr[expr.idx][2]
	if arr[idx][0] != "lambda":
		return None
	x = arr[idx][1]
	idx = arr[idx][2]
	ret = 0
	while True:
		if arr[idx][0] == "symbol" and arr[idx][1] == x:
			return ret
		if arr[idx][0] != "call":
			return None
		if arr[arr[idx][1]][0] != "symbol" or arr[arr[idx][1]][1] != f:
			return None
		ret += 1
		idx = arr[idx][2]

def decompile(expr):
	if expr.idx < 0:
		return "i" + str(-expr.idx)
	for name in churches:
		if alphaEqv(churches[name], expr, []):
			return name
	tmp = decodeNum(expr)
	if tmp is not None:
		return str(tmp)
	args = []
	if expr.arr[expr.idx][0] == "call":
		args.append(expr.arr[expr.idx][2])
		idx = expr.arr[expr.idx][1]
		while True:
			if expr.arr[idx][0] != "call":
				args.append(idx)
				ret = ' '.join([decompile(Expr(expr.arr, i)) for i in reversed(args)])
				return '(' + ret + ')'
			args.append(expr.arr[idx][2])
			idx = expr.arr[idx][1]
	elif expr.arr[expr.idx][0] == "lambda":
		params = [expr.arr[expr.idx][1]]
		idx = expr.arr[expr.idx][2]
		while True:
			if expr.arr[idx][0] != "lambda":
				params = '(' + ' '.join(map(lambda x: 'v'+str(x), params)) + ')'
				return "(lambda %s %s)" % (params, decompile(Expr(expr.arr, idx)))
			params.append(expr.arr[idx][1])
			idx = expr.arr[idx][2]
	elif expr.arr[expr.idx][0] == "symbol":
		return "v" + str(expr.arr[expr.idx][1])
	assert False

if __name__ == '__main__':
	updateChurches()
	chall = loadLambda(0x4040A0, 0x30A0, 0x1BFA8//0x18)
	res0, res1 = [], []
	print(decompile(chall))

	flag = solve(
		np.array([[6,5,5],[5,7,5],[2,3,3]]),
		np.array([1307,1341,781]))
	print("".join(map(lambda x : chr(int(x)), flag)))