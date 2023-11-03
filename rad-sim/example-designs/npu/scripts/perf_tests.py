import os
from os import listdir, chdir
from os.path import isfile, join
import sys
import subprocess

# Define colors for printing
class colors:
	PASS = '\x1b[42m'
	FAIL = '\x1b[41m'
	BOLD = '\033[1m'
	RESET = '\033[0;0m'

# Set default values
num_tiles = 7
num_sectors = 10
num_threads = 4
num_dpes = 40
num_lanes = 40
vrf_depth = 512
mrf_depth = 1024
instances = 1
qor_tolerance = 10.00 # 10 percent
runtime_tolerance = 50.00 # 50 percent

# Parse command line arguments
if('-t' in sys.argv):
	if(sys.argv.index('-t') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -t argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		num_tiles = int(sys.argv[sys.argv.index('-t') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -t argument!" + bcolors.RESET)
		sys.exit(1)

if('-s' in sys.argv):
	if(sys.argv.index('-s') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -s argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		num_sectors = int(sys.argv[sys.argv.index('-s') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -s argument!" + bcolors.RESET)
		sys.exit(1)

if('-th' in sys.argv):
	if(sys.argv.index('-th') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -th argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		num_threads = int(sys.argv[sys.argv.index('-th') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -th argument!" + bcolors.RESET)
		sys.exit(1)

if('-d' in sys.argv):
	if(sys.argv.index('-d') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -d argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		num_dpes = int(sys.argv[sys.argv.index('-d') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -d argument!" + bcolors.RESET)
		sys.exit(1)

if('-l' in sys.argv):
	if(sys.argv.index('-l') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -l argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		num_lanes = int(sys.argv[sys.argv.index('-l') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -l argument!" + bcolors.RESET)
		sys.exit(1)

if('-vd' in sys.argv):
	if(sys.argv.index('-vd') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -vd argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		vrf_depth = int(sys.argv[sys.argv.index('-vd') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -vd argument!" + bcolors.RESET)
		sys.exit(1)

if('-md' in sys.argv):
	if(sys.argv.index('-md') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -md argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		mrf_depth = int(sys.argv[sys.argv.index('-md') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -md argument!" + bcolors.RESET)
		sys.exit(1)

if('-i' in sys.argv):
	if(sys.argv.index('-i') + 1 >= len(sys.argv)):
		print(bcolors.FAIL + "\nInvalid -i argument!" + bcolors.RESET)
		sys.exit(1)
	try:
		instances = int(sys.argv[sys.argv.index('-i') + 1])
	except ValueError:
		print(bcolors.FAIL + "\nInvalid -i argument!" + bcolors.RESET)
		sys.exit(1)

sim_arguments = ['python', '-t', str(num_tiles), '-s', str(num_sectors), '-d', str(num_dpes), '-l', str(num_lanes), \
	'-vd', str(vrf_depth), '-md', str(mrf_depth), '-th', str(num_threads), '-perfsim']

keyword = ''
if ('--run_test' in sys.argv):
	keyword = sys.argv[sys.argv.index('--run_test')+1]

# Get list of existing workloads
path = './workloads/'
workloads = [f for f in listdir(path) if isfile(join(path, f))]
workloads = [f for f in workloads if keyword in f]
workloads.sort()
for i in range(len(workloads)):
	workloads[i] = workloads[i].split('.')[0]

# Parse baseline results
baseline_results = {}
baseline = open('../scripts/perf_baseline', 'r')
for line in baseline:
	split_line = line.split(' ')
	baseline_results[split_line[0]] = (float(split_line[1]), float(split_line[2]))

print(colors.BOLD + '{:<35}    {:<4}    {:<4}    {:<5}    {:<7}    {:<8}    {:<11}    {:<20}'.format('WORKLOAD', 'TEST', 'PERF', 'TOPS', 'QoR', 'Cycles', 'Runtime(s)', 'Speed(s/cyc)') + colors.RESET)

chdir('../compiler')
first_flag = True
cycles = ''
global_failed = False
for workload in workloads:
	subprocess.call(['cp', '../scripts/workloads/'+workload+'.py', './'], shell=False)
	sys.stdout.write('{:<35}    '.format(workload))
	sys.stdout.flush()
	outfile = open('../scripts/reports/'+workload+'_perf.rpt', 'w')
	if (first_flag):
		call_args = sim_arguments
		call_args.insert(1, workload+'.py')
		call_args.append('-is_first')
		subprocess.call(call_args, stdout=outfile, stderr=outfile, shell=False)
		first_flag = False
	else:
		call_args = sim_arguments
		call_args.insert(1, workload+'.py')
		subprocess.call(call_args, stdout=outfile, stderr=outfile, shell=False)
	rptfile = open('../scripts/reports/'+workload+'_perf.rpt', 'r')
	parse_perf_res = False

	# Parse File
	tops = 0
	cycles = 0
	runtime = 0
	speed = 0
	correctness = False
	for line in rptfile:
		if (parse_perf_res and ('Running simulation ... ' in line)):
			args = line.split()
			if('PASSED' in args[3]):
				result = args[10]
				tops = float(result) * instances
				cycles = int(args[4][1:])
				correctness = True	
			else:
				correctness = False
		elif (parse_perf_res and ('Simulation took' in line)):
			args = line.split()
			args = args[2].split('m')
			runtime = int(args[0]) * 60.0
			args = args[1].split('s')
			runtime = runtime + float(args[0])
			speed = runtime/cycles
		elif 'SystemC Performance Simulation' in line:
			parse_perf_res = True

	# Write Results
	if(not parse_perf_res):
		print(colors.FAIL + 'FAIL' + colors.RESET)
		global_failed = True
	else:
		if(correctness):
			print(colors.PASS + 'PASS' + colors.RESET, end='    ') # Correctness PASS
			if workload in baseline_results:
				qor_comparison_to_baseline = ((tops/baseline_results[workload][0])-1) * 100
				runtime_comparison_to_baseline = ((runtime/baseline_results[workload][1])-1) * 100
				if (abs(qor_comparison_to_baseline) < qor_tolerance and abs(runtime_comparison_to_baseline) < runtime_tolerance):
					print(colors.PASS + 'PASS' + colors.RESET, end='    ') # Performance PASS
				else:
					print(colors.FAIL + 'FAIL' + colors.RESET, end='    ') # Performance FAIL
					global_failed = True
				if qor_comparison_to_baseline >= 0:
					print ('{:<5}    +{:<5.2f}%    {:<8}    {:<11.2f}    {:<20.2f}'.format(tops, qor_comparison_to_baseline, cycles, runtime, speed))
				else:
					print ('{:<5}     {:<5.2f}%    {:<8}    {:<11.2f}    {:<20.2f}'.format(tops, qor_comparison_to_baseline, cycles, runtime, speed))
			else:
				print ('    ', end='    ') # No Performance Result
				print ('{:<5}    {:<7}    {:<8}    {:<11.2f}    {:<20.2f}'.format(tops, 'N/A', cycles, runtime, speed))
		else:
			print(colors.FAIL + 'FAIL' + colors.RESET) # Correctness FAIL
			global_failed = True

	subprocess.call(['rm', workload+'.py'], shell=False)

exit(global_failed)