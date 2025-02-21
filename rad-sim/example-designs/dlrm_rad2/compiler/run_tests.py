import sys
import os
import subprocess
import glob
import shutil

models = ['small', 'large']
flit_widths = [131]
vc_buffer_sizes = [8, 16]
mvm_configs = [16, 32]
num_inputs = [1, 256]

root_dir = '/home/andrew/repos/rad-flow-dev/'

report_file = open('report.csv', 'a')
report_file.write('flit_width, vc_buf_size, mvm_lanes, model, num_inputs, cycles\n')

# Go to the RAD flow root directory
os.chdir(root_dir)

for fw in flit_widths:
    for vc in vc_buffer_sizes:
        # For every variation of the flit width and buffer size
        # change the rad-flow.config file and run config script
        if (fw == 131):
            pw = 82
        elif (fw == 195):
            pw = 146
        else:
            pw = 274

        config_file = open(root_dir+'rad-flow.config', 'r')
        lines = config_file.readlines()
        config_file.close()
        for i in range(len(lines)):
            if 'noc_payload_width' in lines[i]:
                lines[i] = 'noc_payload_width = ['+str(pw)+']\n'
            elif 'noc_vc_buffer_size' in lines[i]:
                lines[i] = 'noc_vc_buffer_size = ['+str(vc)+']\n'
        config_file = open(root_dir+'rad-flow.config', 'w')
        config_file.writelines(lines)
        config_file.close()
        os.chdir(root_dir+'scripts/')
        subprocess.run(['python', 'config.py'])

        for mvm in mvm_configs:
            for model in models:
                for inputs in num_inputs:
                    name = 'fw' + str(fw) + '_vc' + str(vc) + '_mvm' + str(mvm) + \
                      '_' + str(model) + '_' + str(inputs)
                    report_file.write(str(fw) + ', ' + 
                                      str(vc) + ', ' + 
                                      str(mvm) + ', ' + 
                                      str(model) + ', ' + 
                                      str(inputs) + ', ')
                    report_file.flush()
                    print(name)
                    
                    print('Creating reports directory ... ', end='', flush=True)
                    reports_path = root_dir + 'rad-sim/example-designs/dlrm/compiler/reports/' + name
                    if not os.path.exists(reports_path):
                        os.makedirs(reports_path)
                    print('Done')
                    
                    
                    # Run dlrm compiler script
                    print('Running DLRM compiler script ... ', end='', flush=True)
                    os.chdir(root_dir+'rad-sim/example-designs/dlrm/compiler/')
                    subprocess.run(['python', 'dlrm.py', 
                                    '-l', str(mvm), 
                                    '-n', str(inputs), 
                                    '-m', 'ab_'+model+'.csv'])
                    print('Done')
                    
                    # Build RAD-Sim
                    print('Building RAD-Sim ... ', end='', flush=True)
                    os.chdir(root_dir+'rad-sim/build/')
                    run_log = open(reports_path + '/radsim.log', 'w')
                    subprocess.run(['make'], stdout=run_log, stderr=run_log)
                    print('Done')

                    # Run RAD-Sim
                    print('Running RAD-Sim ... ', end='', flush=True)
                    run_out = subprocess.run(['./sim/build/system'], 
                                                      stdout=run_log, 
                                                      stderr=run_log, timeout=300)
                    print('Done')
                    run_log.close()
                    
                    # Parse run log
                    read_run_log = open(reports_path + '/radsim.log', 'r')
                    lines = read_run_log.readlines()
                    flag = False
                    for line in lines:
                        if 'PASSED' in line:
                            print('Simulation Passed! ', end='')
                            flag = True
                        if 'Simulated' in line:
                            line = line.split()
                            cycles = line[1]
                            print(str(cycles) + ' cycles')
                            report_file.write(str(cycles) + '\n')
                            report_file.flush()
                    if not flag:
                        print('Something Wrong!')
                        report_file.write('\n')
                        report_file.flush()
                    read_run_log.close()


                    # Copy dramsim reports
                    print('Copying DRAMsim3 reports ... ', end='', flush=True)
                    files = glob.iglob(os.path.join(root_dir+'rad-sim/logs', '*.txt'))
                    for file in files:
                        if (os.path.isfile(file)):
                            shutil.copy2(file, reports_path + '/')
                    print('Done')

                    # Plot and save as html
                    print('Ploting and saving trace ... ', end='', flush=True)
                    os.chdir(root_dir+'rad-sim/example-designs/dlrm/compiler/')
                    subprocess.run(['python', root_dir+'rad-sim/example-designs/dlrm/compiler/plot.py', root_dir+'rad-sim/', reports_path])
                    print('Done')
                    print('---------------------------')
                    

report_file.close()