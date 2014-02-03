import subprocess
import sys
import os

usage = 'test.py <type> <objectfile> <nproc> <rows> <columns> <iterations>'

if len(sys.argv) != 7:
  print usage

p_type = sys.argv[1]
o_file = sys.argv[2]
nproc = sys.argv[3]
rows = sys.argv[4]
cols = sys.argv[5]
iters = sys.argv[6]

input_file = 'input' + rows + 'x' + cols
output_file = 'output' + rows + 'x' + cols


gen_comm = './rand_gen ' + rows + ' ' + cols + ' > ' + input_file
os.system(gen_comm)

mpi_call = 'mpirun -np ' + nproc + ' ' + p_type + '/' + o_file + ' ' + rows + ' ' + cols + ' ' + iters + ' -f ' + input_file + ' > ' + output_file
cilk_call = p_type + '/' + o_file + ' --nproc ' + nproc + ' ' + rows + ' ' + cols + ' ' + iters + ' -f ' + input_file + ' > ' + output_file
omp_call = p_type + '/' + o_file + ' ' + rows + ' ' + cols + ' ' + iters + ' -p ' + nproc + ' -f ' + input_file + ' > ' + output_file
seq_call = './row ' + rows + ' ' + cols + ' ' + iters + ' -f ' + input_file + ' > ' + output_file + '_seq'

if p_type == 'mpi':
  print mpi_call
  os.system(mpi_call)
elif p_type == 'cilk':
  print cilk_call
  os.system(cilk_call)
elif p_type == 'omp':
  print omp_call
  os.system(omp_call)


os.system(seq_call)

p = subprocess.Popen(['diff', output_file, output_file+'_seq'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()


if err == '' and out == '':
  print 'test successful'
else:
  print 'TEST FAILED!!!!'
os.remove(input_file)
os.remove(output_file)
os.remove(output_file+'_seq')
