import time, sys, math, random, glob
import os.path


data_directory    = "data"

#data_directory    = os.path.join(args.data_root, args.data_folder)

def get_random_training_filename_pair():
    input_voice_mask = random.randint(1, 14)
    free_voices = [];
    for i in range(4):
        if (input_voice_mask & (1 << i)) == 0:
            free_voices.append(i)
      
    output_voice_mask = 1 << (free_voices[random.randint(0, len(free_voices)-1)]);

    print("Audio: {} MIDI{}".format(bin(input_voice_mask), bin(output_voice_mask)))
    wavs = glob.glob("dataset/audio/*_{}.wav".format(input_voice_mask));
    wav =  wavs[random.randint(0, len(wavs)-1)];
    print(wav);
    
    mid = os.path.basename(wav)
    mid = mid.replace("_{}.wav".format(input_voice_mask), "_{}.mid".format(output_voice_mask))
    mid = os.path.join("dataset/MIDI", mid)
    print(mid);

get_random_training_filename_pair()
