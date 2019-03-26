#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

bool read_input(char *buf, size_t len);

template<size_t len>
class StringBuffer
{
private:
	char buffer[len];
public:
	void clear()
	{
		memset(buffer, 0, len);
	}
	void print()
	{
		printf("%s", buffer);
	}
	StringBuffer()
	{
		clear();
	}
	virtual ~StringBuffer()
	{
		clear();
	}
	bool read_string()
	{
		return read_input(buffer, len);
	}
};

constexpr size_t BUF_SIZE = 0x10;
constexpr size_t STRING_SIZE = BUF_SIZE - 1;
constexpr size_t MAX_NUM = 0x20;
constexpr size_t NAME_SIZE = BUF_SIZE;

StringBuffer<NAME_SIZE> name;

StringBuffer<BUF_SIZE>* data[MAX_NUM];
size_t data_num[MAX_NUM];

void __attribute__ ((noinline)) leak()
{
	asm("mov rax, 0x602018\n"
		"mov rax, [rax]\n"
		"mov rdx, rax\n"
		"push 0xa7025\n"
		"mov rsi,rsp\n"
		"xor rdi,rdi\n"
		"call __printf_chk\n"
		"add rsp,8");
}

void menu()
{
	puts("1. malloc");
	puts("2. free");
	puts("3. new");
	puts("4. delete");
	puts("5. puts");
	puts("6. exit");
	if (printf(">> ") == (int)0xdeadbeef)
		leak();
}

//return true if the buffer has been filled fully
bool read_input(char* buf, size_t len)
{
	if (len == 0) return true;
	size_t i;
	for (i = 0; i < len - 1; i++)
	{
		ssize_t ret = read(STDIN_FILENO, buf + i, 1);
		if (ret != 1)
			exit(-1);
		if (buf[i] == '\n')
			break;
	}
	buf[i] = 0;
	return i == len - 1;
}

size_t read_long()
{
	constexpr size_t LONG_BUF_LEN = 0x10;
	char buf[LONG_BUF_LEN];
	read_input(buf, LONG_BUF_LEN);
	return strtol(buf, nullptr, 10);
}

template<typename T>
void alloc_sb(T allocator)
{
	puts("Please input length of the string");
	size_t length = read_long();
	if (length > 0x400)
		exit(-1);
	size_t num_blocks = (length + STRING_SIZE - 1) / STRING_SIZE;
	size_t i;

	//find the empty entry
	for (i = 0; i < MAX_NUM; ++i)
	{
		if (data[i] == nullptr)
			break;
	}
	if (i == MAX_NUM)
		exit(-1);

	//allocate
	data[i] = allocator(num_blocks);
	data_num[i] = num_blocks;

	//input
	puts("Please input the string");
	for (size_t j = 0; j < num_blocks; ++j)
	{
		bool ret = data[i][j].read_string();
		if (!ret)
			break;
	}
}

StringBuffer<BUF_SIZE>* alloc_malloc(size_t num_blocks)
{
	StringBuffer<BUF_SIZE>* ret = (StringBuffer<BUF_SIZE>*)malloc(num_blocks * sizeof(StringBuffer<BUF_SIZE>));
	for (size_t j = 0; j < num_blocks; ++j)
	{
		ret[j].clear();
	}
	return ret;
}

StringBuffer<BUF_SIZE>* alloc_new(size_t num_blocks)
{
	return new StringBuffer<BUF_SIZE>[num_blocks]();
}

template<typename T>
void oper_sb(T operation)
{
	puts("Please input index of the string");
	size_t idx = read_long();
	if (idx >= MAX_NUM)
		exit(-1);
	if (data[idx] == nullptr)
	{
		puts("The string does not exit");
	}
	else
	{
		operation(idx);
	}
}

//pre: idx is valid one
void oper_delete(size_t idx)
{
	delete[] data[idx];
	data[idx] = nullptr;
}

void oper_free(size_t idx)
{
	free(data[idx]);
	data[idx] = nullptr;
}

void oper_puts(size_t idx)
{
	for (size_t j = 0; j < data_num[idx]; ++j)
	{
		data[idx][j].print();
	}
	printf("\n");
}

int main(int argc, char const *argv[])
{
	setvbuf(stdout, nullptr, 2, 0);
	setvbuf(stderr, nullptr, 2, 0);

	printf("Please input your name: ");
	name.read_string();

	while(true)
	{
		menu();
		size_t opt = read_long();
		switch(opt)
		{
			case 0:
			break;
			case 1:
			alloc_sb(alloc_malloc);
			break;
			case 2:
			oper_sb(oper_free);
			break;
			case 3:
			alloc_sb(alloc_new);
			break;
			case 4:
			oper_sb(oper_delete);
			break;
			case 5:
			oper_sb(oper_puts);
			break;
			default:
			return 0;
		}
	}
	return 0;
}
//g++ -masm=intel -s -O2 -Wall -std=c++11 main.cpp -o candcpp
