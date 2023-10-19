import os
from os import listdir, chdir
from os.path import isfile, join
import sys
import subprocess

test_mode = 'all'
test_dirs = ['rad_1', 'rad_2', 'rad_3']
test_idx = 0
num_args = len(sys.argv)

if (num_args == 1):
	test_mode = 'all'
elif(num_args == 2):
	input_arg = sys.argv[1]
	if (input_arg == 'all'):
		test_mode = 'all'
	else:
		test_mode = 'single'
		try:
			test_idx = int(sys.argv[1]) - 1
			if (test_idx < 0):
				print('Invalid argument(s): Test number should be greater than zero!')
				sys.exit(1)
			elif (test_idx >= len(test_dirs)):
				print('Invalid argument(s): Only ' + str(len(test_dirs)) + ' tests exist!')
				sys.exit(1)
		except ValueError:
			print('Invalid argument(s): Given argument has to be \"all\" or the number of the test you want to run!')
			sys.exit(1)
else:
	print('Invalid argument(s): Expecting at most 1 command line argument for this script!')
	sys.exit(1)

# Run tests with specified configurations
if (test_mode == 'all'):
	for t in range(len(test_dirs)):
		current_test = test_dirs[t]
		print('RUNNING TEST ' + current_test + ' ...')
		subprocess.call(['cp', './'+current_test+'/rad-flow.config', '../../../rad-flow.config'], shell=False)
		subprocess.call(['cp', './'+current_test+'/npu.clks','../../example-designs/npu/npu.clks'], shell=False)
		subprocess.call(['cp', './'+current_test+'/npu.place','../../example-designs/npu/npu.place'], shell=False)
		subprocess.call(['cp', './'+current_test+'/interface_defines.hpp','../../example-designs/npu/modules/interface_defines.hpp'], shell=False)
		chdir('../../../scripts')
		subprocess.call(['python', 'config.py'], shell=False)
		chdir('../rad-sim/example-designs/npu/scripts')
		if (current_test == 'rad_1'):
			test_args = ['python', 'perf_tests.py', '-s', '5', '-vd', '512', '-md', '1024', '-th', '4']
		elif (current_test == 'rad_2'):
			test_args = ['python', 'perf_tests.py', '-s', '10', '-vd', '1024', '-md', '2048', '-th', '4']
		elif (current_test == 'rad_3'):
			test_args = ['python', 'perf_tests.py', '-s', '10', '-vd', '1024', '-md', '2048', '-i', '4', '-th', '1']
		subprocess.call(test_args, shell=False)
		chdir('../../../test/npu-rads')
else:
	current_test = test_dirs[test_idx]
	print('RUNNING TEST ' + current_test + ' ...')
	subprocess.call(['cp', './'+current_test+'/rad-flow.config', '../../../rad-flow.config'], shell=False)
	subprocess.call(['cp', './'+current_test+'/npu.clks','../../example-designs/npu/npu.clks'], shell=False)
	subprocess.call(['cp', './'+current_test+'/npu.place','../../example-designs/npu/npu.place'], shell=False)
	subprocess.call(['cp', './'+current_test+'/interface_defines.hpp','../../example-designs/npu/modules/interface_defines.hpp'], shell=False)
	chdir('../../../scripts')
	subprocess.call(['python', 'config.py'], shell=False)
	chdir('../rad-sim/example-designs/npu/scripts')
	if (current_test == 'rad_1'):
		test_args = ['python', 'perf_tests.py', '-s', '5', '-vd', '512', '-md', '1024', '-th', '4']
	elif (current_test == 'rad_2'):
		test_args = ['python', 'perf_tests.py', '-s', '10', '-vd', '1024', '-md', '2048', '-th', '4']
	elif (current_test == 'rad_3'):
		test_args = ['python', 'perf_tests.py', '-s', '10', '-vd', '1024', '-md', '2048', '-i', '4', '-th', '1']
	subprocess.call(test_args, shell=False)
	chdir('../../../test/npu-rads')

# Copy back the original files
subprocess.call(['cp', '../../example-designs/npu/default_configs/rad-flow.config', '../../../rad-flow.config'], shell=False)
subprocess.call(['cp', '../../example-designs/npu/default_configs/npu.clks', '../../example-designs/npu/npu.clks'], shell=False)
subprocess.call(['cp', '../../example-designs/npu/default_configs/npu.place', '../../example-designs/npu/npu.place'], shell=False)
subprocess.call(['cp', '../../example-designs/npu/default_configs/interface_defines.hpp', '../../example-designs/npu/modules/interface_defines.hpp'], shell=False)

