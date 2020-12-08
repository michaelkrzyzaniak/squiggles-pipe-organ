import argparse
import time, sys, math
import os.path
from models.Multiple_F0 import Multiple_F0

default_sessions_root     = "sessions"
default_data_root         = "data"
default_num_batches       = 1000
default_learning_rate     = 1e-4
default_use_cpu           = "False"
default_save_every        = 50
default_gold_standard     = ""
examples_per_batch        = 50

parser = argparse.ArgumentParser(description='Wave RNN Network')
parser.add_argument('--sessions_root' , type=str, default=default_sessions_root, help='sessions folder. Default: ' + default_sessions_root + '.')
parser.add_argument('--session_folder', type=str, default=None                 , help='Required: the name of the current session. The session will be restored if it exists, or it will be created in sessions_root', required=True)
parser.add_argument('--num_batches'   , type=int, default=default_num_batches  , help='The number of iterations to train. Default: ' + str(default_num_batches) + '.')
parser.add_argument('--learning_rate' , type=float, default=default_learning_rate, help='The global learning rate (Adam). Default: ' + str(default_learning_rate) + '.')
parser.add_argument('--data_root'     , type=str, default=default_data_root    , help='data root folder. Default: ' + str(default_data_root) + '.')
parser.add_argument('--data_folder'   , type=str, default=None                 , help='The folder in data_root where your data lives. Default: ' + str(default_sessions_root) + '.', required=True)
parser.add_argument('--save_every'    , type=int, default=default_save_every   , help='How often to save checkpoints' + str(default_save_every) + '.')
parser.add_argument('--use_cpu'       , type=str, default=default_use_cpu      , help='Should use cpu instead of gpu? Default: ' + str(default_use_cpu) + '.')
parser.add_argument('--gold_standard' , type=str, default=default_gold_standard, help='Gold standard to sample? Default: ' + str(default_gold_standard) + '.')

args = parser.parse_args()

session_directory = os.path.join(args.sessions_root, args.session_folder)
data_directory    = os.path.join(args.data_root, args.data_folder)

if not os.path.exists(args.sessions_root):
    os.makedirs(args.sessions_root)
if not os.path.exists(session_directory):
    os.makedirs(session_directory)

f0 = Multiple_F0(session_directory, data_directory, args.use_cpu != "False")
f0.restore_if_checkpoint_exists(session_directory)

if args.use_cpu == "False" :
  f0 = f0.cuda();
else:
  f0 = f0.cpu();


if args.gold_standard != "":
    #f0.reverse_synthesize_gold_standard(args.gold_standard)
    f0.sample()
    exit()

if not os.path.exists(data_directory):
    print ("No such data directory " + data_directory)
    exit();

f0.train_model(args.num_batches, examples_per_batch, args.save_every, args.learning_rate)
#f0.reverse_synthesize_gold_standard(160)

