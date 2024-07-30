import sys
import math
import os
import glob
import numpy as np

# Defining constants
native_dim = 16
num_test_inputs = 32
num_noc_routers = 16

if ('-h' in sys.argv or '--help' in sys.argv):
  print('python gen_testcase.py <num_layers> <input_dim> [<hidden_dims>] [<mvms_per_layer>]')
  exit(1)

# Parse command line arguments
num_layers = int(sys.argv[1])
print('Number of layers = ' + str(num_layers))
input_dim = int(sys.argv[2])
print('Input dimension = ' + str(input_dim))
hidden_dims = np.zeros(shape=num_layers, dtype=int)
print('Hidden dimensions = [ ', end='')
for i in range(3, 3 + num_layers):
  hidden_dims[i-3] = int(sys.argv[i])
  print(str(hidden_dims[i-3]) + ' ', end='')
print(']')
num_mvms = np.ones(shape=num_layers, dtype=int)
print('Number of MVMs = [ ', end='')
for i in range(3 + num_layers, 3 + 2 * num_layers):
  num_mvms[i-3-num_layers] = int(sys.argv[i])
  print(str(num_mvms[i-3-num_layers]) + ' ', end='')
print(']')

# Generate random padded weight matrices
padded_weights = []
for l in range(num_layers):
  num_mvms_in = num_mvms[l]
  if (l == num_layers-1):
    num_mvms_out = num_mvms[0]
  else:
    num_mvms_out = num_mvms[l+1]
  if (l == 0):
    layer_input_dim = input_dim
  else:
    layer_input_dim = hidden_dims[l-1]
  padded_dimx = int(math.ceil(layer_input_dim * 1.0 / native_dim / num_mvms_in) * native_dim * num_mvms_in)
  padded_dimy = int(math.ceil(hidden_dims[l] * 1.0 / native_dim / num_mvms_out) * native_dim * num_mvms_out)
  padded_weights.append(np.zeros(shape=(padded_dimy, padded_dimx), dtype=int))
  padded_weights[l][:hidden_dims[l], :layer_input_dim] = np.random.randint(-2, 2, size=(hidden_dims[l], layer_input_dim))

# Prepare weight MIFs directory
if(not(os.path.exists('./weight_mifs'))):
  os.mkdir('weight_mifs')
else:
  files = glob.glob('weight_mifs/*.mif')
  for file in files:
    os.remove(file)

# Write weight MIFs
for l in range(num_layers):
  layer_mvms = num_mvms[l]
  mvm_idx = 0
  limx = int(padded_weights[l].shape[1] / native_dim)
  limy = int(padded_weights[l].shape[0] / native_dim)
  mifs = []
  for m in range(layer_mvms):
    mifs.append([])
    for d in range(native_dim):
      mifs[m].append(open('weight_mifs/layer'+str(l)+'_mvm'+str(m)+'_dot'+str(d)+'.mif', 'w'))
  
  for i in range(limx):
    for j in range(limy):
      for d in range(native_dim):
        for e in range(native_dim):
          mifs[mvm_idx][d].write(str(padded_weights[l][(j * native_dim) + d][(i * native_dim) + e]) + ' ')
        mifs[mvm_idx][d].write('\n')
    if (mvm_idx == layer_mvms-1):
      mvm_idx = 0
    else:
      mvm_idx = mvm_idx + 1

  for mvm_mifs in mifs:
    for mif in mvm_mifs:
      mif.close()

# Prepare instruction MIFs directory
if(not(os.path.exists('./inst_mifs'))):
  os.mkdir('inst_mifs')
else:
  files = glob.glob('inst_mifs/*.mif')
  for file in files:
    os.remove(file)

# Generate instruction MIFs
# en, jump, reduce, accum, accum_en, release, raddr, last, dest_layer, dest_mvm
for l in range(num_layers):
  layer_mvms = num_mvms[l]
  limx = int(padded_weights[l].shape[1] / native_dim / layer_mvms)
  limy = int(padded_weights[l].shape[0] / native_dim)
  for m in range(layer_mvms):
    inst_mif = open('inst_mifs/layer'+str(l)+'_mvm'+str(m)+'.mif', 'w')
    for i in range(limx):
      for j in range(limy):
        if ((l == num_layers - 1) and (m == layer_mvms - 1)):
          dest_layer = 0
          dest_mvm = 0
        elif (m == layer_mvms - 1):
          dest_layer = l + 2
          dest_mvm = j % num_mvms[l+1]
        else:
          dest_layer = l + 1
          dest_mvm = m + 1
        inst_mif.write('1 0 ') # en, jump
        if (m == 0 or i < limx-1):
          inst_mif.write('0 ') # reduce
        else:
          inst_mif.write('1 ') # reduce
        if (i == 0):
          inst_mif.write(str(j) + ' 0 ') # accum, accum_en
        else:
          inst_mif.write(str(j) + ' 1 ') # accum, accum_en
        if (i == limx-1):
          inst_mif.write('1 ') # release
        else:
          inst_mif.write('0 ') # release
        inst_mif.write(str(i * limy + j) + ' ') # raddr
        if (j == limy-1):
          inst_mif.write('1 ') # last
        else:
          inst_mif.write('0 ') # last
        inst_mif.write(str(dest_layer) + ' ' + str(dest_mvm) + '\n') # dest_layer, dest_mvm
    inst_mif.write('1 1 0 0 0 0 0 0 0 0\n')
    inst_mif.close()

# Prepare input MIFs directory
if(not(os.path.exists('./input_mifs'))):
  os.mkdir('input_mifs')
else:
  files = glob.glob('input_mifs/*.mif')
  for file in files:
    os.remove(file)

# Generate test input MIFs
padded_input_dim = int(math.ceil(input_dim * 1.0 / native_dim / num_mvms[0]) * native_dim * num_mvms[0])
test_inputs = np.zeros(shape=(num_test_inputs, padded_input_dim), dtype=int)
test_inputs[:, :input_dim] = np.random.randint(-2, 2, size=(num_test_inputs, input_dim))
input_files = []
for i in range(num_mvms[0]):
  input_files.append(open('input_mifs/inputs_mvm' + str(i) + '.mif', 'w'))
for i in range(num_test_inputs):
  for c in range(int(padded_input_dim / native_dim)):
    for e in range(native_dim):
      input_files[c % num_mvms[0]].write(str(test_inputs[i][(c * native_dim) + e]) + ' ')
    input_files[c % num_mvms[0]].write('\n')
for file in input_files:
  file.close()

# Compute test outputs
test_inputs = np.transpose(test_inputs)
test_outputs = np.dot(padded_weights[0], test_inputs)
#test_outputs = np.maximum(test_outputs, np.zeros(shape=test_outputs.shape, dtype=int))
for l in range(1, num_layers):
  test_outputs = np.dot(padded_weights[l], test_outputs)
  #test_outputs = np.maximum(test_outputs, np.zeros(shape=test_outputs.shape, dtype=int))
test_outputs = np.transpose(test_outputs)

# Generate test output MIFs
output_file = open('./golden_outputs.mif', 'w')
for o in range(test_outputs.shape[0]):
  for c in range(int(test_outputs.shape[1] / native_dim)):
    for e in range(native_dim):
      output_file.write(str(test_outputs[o][(c * native_dim) + e]) + ' ')
    output_file.write('\n')
output_file.close()

# Generate layer/MVM configuration
config_file = open('./layer_mvm_config', 'w')
config_file.write(str(num_layers) + ' ')
for mvm_count in num_mvms:
  config_file.write(str(mvm_count) + ' ')
config_file.close()

# Generate placement and clock constraint files
placement_file = open('../mlp.place', 'w')
clocks_file = open('../mlp.clks', 'w')
total_num_blocks = 0
for mvm_count in num_mvms:
  total_num_blocks = total_num_blocks + mvm_count
total_num_blocks = total_num_blocks + num_mvms[0] + 1
if (total_num_blocks > num_noc_routers):
  print("Number of blocks is more than NoC routers!")
  exit(1)
router_ids = np.random.choice(num_noc_routers, size=total_num_blocks, replace=False)
idx = 0
for l in range(num_layers):
  for m in range(num_mvms[l]):
    mvm_name = 'layer' + str(l) + '_mvm' + str(m)
    placement_file.write(mvm_name + ' 0 ' + str(router_ids[idx]) + ' axis\n')
    idx = idx + 1
    clocks_file.write(mvm_name + ' 0 0\n')
for m in range(num_mvms[0]):
  dispatcher_name = 'input_dispatcher' + str(m)
  placement_file.write(dispatcher_name + ' 0 ' + str(router_ids[idx]) + ' axis\n')
  idx = idx + 1
  clocks_file.write(dispatcher_name + ' 0 0\n')
placement_file.write('output_collector 0 ' + str(router_ids[idx]) + ' axis\n')
idx = idx + 1
clocks_file.write('output_collector 0 0\n')

placement_file.write('portal_inst 0 16 axis\n')
clocks_file.write('portal_inst 0 0\n')

placement_file.close()
clocks_file.close()

'''for l in range(num_layers):
  print('Layer ' + str(l) + ':')
  for y in range(padded_weights[l].shape[0]):
    for x in range(padded_weights[l].shape[1]):
      print('{:>4} '.format(padded_weights[l][y][x]), end='')
    print('')
  print('---------------------------------------')

for i in range(num_test_inputs):
  print('Input ' + str(i) + ':')
  for x in range(test_inputs.shape[0]):
    print('{:>4} '.format(test_inputs[x][i]), end='')
  print('\nOutput ' + str(i) + ':')
  for x in range(test_outputs.shape[0]):
    print('{:>4} '.format(test_outputs[i][x]), end='')
  print('\n---------------------------------------')'''

