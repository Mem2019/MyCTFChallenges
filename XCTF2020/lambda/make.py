import os

for i in range(0,1):
	# randomly generate flag and lambda expression
	os.system("python3 compiler.py > expr.h")
	# create directory
	dir_name = "chall%u" % i
	os.mkdir(dir_name)
	# generate binary and move it to directory
	os.system("gcc -Wall -s main.c -o lambda")
	os.system("rm expr.h")
	os.system("python3 solve.py > %s/flag" % dir_name)
	os.system("mv lambda ./%s/" % dir_name)
	os.system("cat ./%s/flag | ./%s/lambda" % (dir_name, dir_name))