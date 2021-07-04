import torch
import torch.nn as nn
import torch.nn.functional as F
from   torch import optim
import numpy as np
import librosa, math
import glob
import os.path
import mido
import time
import random
from mido import MidiFile
from mido import MidiTrack
from mido import Message
import soundfile


class LSTM_Harmonizer(nn.Module) :
    #-------------------------------------------------------------------------------------------
    def __init__(self, session_directory, data_directory, use_cpu=False) :
        super(LSTM_Harmonizer, self).__init__()
        
        #inclusive of both
        self.lowest_midi_note = 36
        self.highest_midi_note = 96
        
        self.sequence_length = 127
        
        self.input_size = 2048
        self.hop_size = self.input_size // 2
        self.hidden_size = 256
        self.output_size = self.highest_midi_note - self.lowest_midi_note + 1 + 1 #one extra for silence
        self.num_lstm_layers = 1
        
        self.lstm_input_size = self.hidden_size
        self.lstm_hidden_size = self.hidden_size
        
        #supposedly runs faster with batch_first = false
        self.lstm = nn.LSTM(input_size=self.lstm_input_size, hidden_size=self.lstm_hidden_size, num_layers=self.num_lstm_layers, batch_first=False)
        #self.lstm = torch.nn.RNN(input_size=self.lstm_input_size, hidden_size=self.lstm_hidden_size, num_layers=self.num_lstm_layers);
        #self.lstm = nn.LSTMCell(input_size=self.lstm_input_size, hidden_size=self.lstm_hidden_size)
        #self.fc_out = nn.Linear(self.hidden_size, self.output_size)
        #self.lstm_out_activation = torch.nn.Sigmoid()
        
        #self.lstm_out_activation = torch.nn.LogSoftmax(dim = 1)
        
        #plus 1 for counter
        self.layer_1      = torch.nn.Linear(self.input_size + self.output_size + 1, self.hidden_size);
        self.activation_1 = torch.nn.ReLU()
        self.layer_2      = torch.nn.Linear(self.lstm_hidden_size, self.output_size);
        #self.activation_2 = torch.nn.Sigmoid()
        
        #this helps these get saved and restored correctly.
        self.use_cpu                  = use_cpu
        self.sample_rate              = 44100
        self.session_directory        = session_directory
        self.data_directory           = data_directory
        self.__saved_checkpoint_batch = nn.Parameter(torch.IntTensor([0]), requires_grad=False)
        self.__sample_rate            = nn.Parameter(torch.FloatTensor([self.sample_rate]), requires_grad=False)
        self.model_save_prefix        = "model_"
        
        # display num params
        self.num_params()

    #-------------------------------------------------------------------------------------------
    def save_checkpoint(self, num_batches):
        path = os.path.join(self.session_directory, self.model_save_prefix + str(num_batches).zfill(6) + ".checkpoint")
        self.__saved_checkpoint_batch[0] = num_batches;
        torch.save(self.state_dict(), path);
        #old_checkpoints = glob.glob(os.path.join(self.session_directory, self.model_save_prefix) + "*");
        #for checkpoint in old_checkpoints:
        #    if checkpoint != path:
        #        os.remove(checkpoint);
        print("Saved " + path + " at batch " + str(num_batches));
        
    #-------------------------------------------------------------------------------------------
    def restore_if_checkpoint_exists(self, session_directory) :
        saved_model = glob.glob(os.path.join(session_directory, self.model_save_prefix) + "*.checkpoint")
        if len(saved_model) > 0:
            saved_model = sorted(saved_model)[-1];
            #todo: can't I check whether the state was loaded successfully?
            model = None
            model = torch.load(saved_model, map_location="cpu")
            
            #if torch.cuda.is_available():
            #    model = torch.load(saved_model, map_location='gpu')
            #else:
            #    model = torch.load(saved_model, map_location='cpu')

            self.load_state_dict(model)
            self.session_directory = session_directory
            #self.sample_rate = self.__sample_rate.item()
            print("Restoring checkpoint: {} pretrained with {} batches".format(saved_model, self.__saved_checkpoint_batch[0]))
        else:
            print("Creating Session: " + session_directory)

    #-------------------------------------------------------------------------------------------
    def get_saved_num_batches(self):
        return self.__saved_checkpoint_batch.item()

    #-------------------------------------------------------------------------------------------
    def calculate_audio_features(self, audio):
        n = len(audio)
        #apply hanning window
        audio = np.multiply(audio, np.hanning(n))
        
        #pad to length 2n
        audio = np.pad(audio, (0, n), 'constant', constant_values=(0, 0))
        
        #compute DFT
        spectra = np.fft.rfft(audio)
        
        #cancel noise
        #for i in range(len(spectra)) :
        #    if abs(spectra[i]) < 0.0002 : #-74 dB
        #        spectra[i]= 0+0j
  
        #square it(dft of autocorrelation)
        conjuga = np.conjugate(spectra)
        spectra = np.multiply(spectra, conjuga)
        
        #spectra = np.real(spectra)
        #compute autocorrelation
        spectra = np.fft.irfft(spectra, 2*n)
        audio = spectra[:self.input_size]
        
        #dont normalize audio, tried it, made a lot of crap in the silent sectinos
        #normalize audio
        #max = audio[0]
        #if max > 0 :
        #    audio = np.multiply(audio, 1/max)
        
        return audio
        
    
    #-------------------------------------------------------------------------------------------
    def get_active_MIDI_notes_in_time_range(self, midi, start_secs, end_secs):
        running_time = 0
        active_notes = []
        result = []
        
        for msg in midi:
            running_time += msg.time
            
            if running_time > end_secs:
                break;
    
            elif running_time < start_secs:
                if msg.type == 'note_on':
                    active_notes.append(msg);
                if msg.type == 'note_off':
                  for m in active_notes:
                      if (m.note == msg.note) and (m.channel == msg.channel):
                          active_notes.remove(m)
                  
            else: #start_secs < running_secs < end_secs
                if msg.type == 'note_on':
                    reattack = False
                    for m in active_notes:
                        if (m.note == msg.note) and (m.channel == msg.channel):
                            reattack = True
                            break;
                    if not reattack:
                        active_notes.append(msg)
    
            #if not msg.is_meta
        for m in active_notes:
            result.append(m.note);
    
        return result;

    #-------------------------------------------------------------------------------------------
    def output_notes_to_vector(self, notes):
        vector = np.zeros(self.output_size);
        last_note_index = 0
        
        #rest
        if len(notes) == 0 :
          last_note_index = self.output_size-1
          vector[last_note_index] = 1

        for note in notes:
            while note > self.highest_midi_note:
                note -= 12
            while note < self.lowest_midi_note:
                note += 12
            #++output_array[note]
            last_note_index = note-self.lowest_midi_note
            vector[last_note_index] = 1
        return vector, last_note_index

    #-------------------------------------------------------------------------------------------
    def get_random_training_filename_pair(self, is_training=True):
        training_folder = "Training" if is_training else "Validation"
        audio_directory = os.path.join(self.data_directory, training_folder, "Audio")
        midi_directory  = os.path.join(self.data_directory, training_folder, "MIDI")
        
        #any with at least 2 voices
        input_voice_mask = random.choice([3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15]);
        #Any with at least one missing voice #input_voice_mask = random.randint(1, 14)
        #FOR TOY DATA #input_voice_mask = random.choice([1, 8]);
        
        #get midi file with missing voice
        #free_voices = [];
        #for i in range(4):
        #    if (input_voice_mask & (1 << i)) == 0:
        #        free_voices.append(i)
                
        #output_voice_mask = 1 << (free_voices[random.randint(0, len(free_voices)-1)]);
        ##output_voice_mask = 8 if (input_voice_mask == 1) else 1

        #get midi file with included voice
        included_voices = [];
        for i in range(4):
          if(input_voice_mask & (1 << i)) != 0:
            included_voices.append(i)

        output_voice_mask = 1 << (random.choice(included_voices));

        wavs = glob.glob(audio_directory + "/*_{}.wav".format(input_voice_mask));
        wav =  wavs[random.randint(0, len(wavs)-1)];
    
        mid = os.path.basename(wav)
        mid = mid.replace("_{}.wav".format(input_voice_mask), "_{}.mid".format(output_voice_mask))
        #mid = mid.replace(".wav", ".mid")
        mid = os.path.join(midi_directory, mid)
        return wav, mid
        
    #-------------------------------------------------------------------------------------------
    def get_random_training_sequence_from_file(self, wav_filename, midi_filename):
        wav, sr = librosa.load(wav_filename, sr=self.sample_rate, mono=True)
        sequence_length_in_samples = (self.sequence_length*self.hop_size-1) + self.input_size
        wav_length_in_samples = len(wav)
        if wav_length_in_samples < (wav_length_in_samples):
            print("{} could not be loaded because it is too short.".format(audio_filename))
            return None, None
        
        input_vectors = [];
        start_sample = np.random.randint(self.input_size, wav_length_in_samples-sequence_length_in_samples-self.hop_size-1)
        for i in range(self.sequence_length):
            s = start_sample + (i*self.hop_size)
            input_vectors.append(wav[s : s+self.input_size])
            input_vectors[i] = self.calculate_audio_features(input_vectors[i])
        

        output_vectors = []
        midi_file = MidiFile(midi_filename)
        start_secs = start_sample / sr
        end_secs = (start_sample + self.input_size) / sr
        hop_secs = self.hop_size / sr

        notes = self.get_active_MIDI_notes_in_time_range(midi_file, start_secs, end_secs)
        current_onehot_output, current_output_note = self.output_notes_to_vector(notes)

        counter = 0
        #scan back to see how long this note has been active
        scan_back_start_secs = start_secs - hop_secs;
        scan_back_end_secs   = end_secs - hop_secs;
        while(scan_back_start_secs >= 0) :
          notes = self.get_active_MIDI_notes_in_time_range(midi_file, scan_back_start_secs, scan_back_end_secs)
          scan_back_onehot_output, scan_back_output_note = self.output_notes_to_vector(notes)
          if(scan_back_output_note != current_output_note):
            break
          else:
            ++counter;
          scan_back_start_secs -= start_secs - hop_secs;
          scan_back_end_secs   -= end_secs - hop_secs;
        
        start_secs += hop_secs
        end_secs   += hop_secs


        for i in range(self.sequence_length):
            notes = self.get_active_MIDI_notes_in_time_range(midi_file, start_secs, end_secs)
            next_onehot_output, next_output_note = self.output_notes_to_vector(notes)
            output_vectors.append(next_output_note)
            input_vectors[i] = np.concatenate((input_vectors[i], current_onehot_output, [counter]))
            current_onehot_output = next_onehot_output;
            counter = counter + 1
            if(next_output_note != current_output_note) :
              counter = 0
            current_output_note = next_output_note
            start_secs += hop_secs
            end_secs += hop_secs
    
        return input_vectors, output_vectors

    #-------------------------------------------------------------------------------------------
    def get_random_training_sequence(self, is_training=True):

        wav, mid = self.get_random_training_filename_pair()
        x, y = self.get_random_training_sequence_from_file(wav, mid)
        return x, y

    #-------------------------------------------------------------------------------------------
    def get_random_training_batch(self, examples_per_batch, is_training=True):
        input_data = [];
        output_data = [];
        #training_folder = "Training" if is_training else "Validation"
        #wav_paths = glob.glob(os.path.join(self.data_directory, "Training", "Audio/*.wav"))
        
        for i in range(examples_per_batch):
            wav, mid = self.get_random_training_filename_pair()
            x, y = self.get_random_training_sequence_from_file(wav, mid)
            input_data.append(x)
            output_data.append(y)

        #LSTM wants the indices to be [sequence][batch][inputs]
        #torch.transpose(input_data , 0, 1)
        #torch.transpose(output_data, 0, 1)
        
        #jk, just output [batch][seq][inputs] because it is going into FF, and output of that will have to be separately transposed

        return input_data, output_data
    
    #-------------------------------------------------------------------------------------------
    def num_params(self) :
        parameters = filter(lambda p: p.requires_grad, self.parameters())
        parameters = sum([np.prod(p.size()) for p in parameters]) / 1000000
        print('Trainable Parameters: %.3f million' % parameters)

    #-------------------------------------------------------------------------------------------
    def time_since(self, started) :
        elapsed = time.time() - started
        m = int(elapsed // 60)
        s = int(elapsed % 60)
        if m >= 60 :
            h = int(m // 60)
            m = m % 60
            return str(h) + ":" + str(m) + ":" + str(s).zfill(2)
        else :
            return str(m) + ":" + str(s).zfill(2)
    
    #-------------------------------------------------------------------------------------------
    #https://pytorch.org/tutorials/beginner/nlp/sequence_models_tutorial.html
    def forward(self, x, prev_state) :
    
        hidden = torch.Tensor(len(x), len(x[0]), self.lstm_input_size);
        
        if self.use_cpu == False:
          hidden  = hidden.cuda()
        
        #[batch][sequence][inputs]
        for i in range(len(x)):
          hidden[i] = self.activation_1(self.layer_1(x[i]))
        
        #[sequence][batch][inputs]
        hidden = torch.transpose(hidden , 0, 1)
        
        lstm_out, prev_state = self.lstm(hidden, prev_state)
        #lstm_out, prev_state = self.lstm(hidden.view(x.shape[0], 1, -1), prev_state)
        
        #[batch][sequence][inputs]
        lstm_out = torch.transpose(lstm_out , 0, 1)
        
        output = torch.Tensor(len(lstm_out), len(lstm_out[0]), self.output_size);
        if self.use_cpu == False:
          output  = output.cuda()
  
        for i in range(len(lstm_out)):
          output[i] = self.layer_2(lstm_out[i])

        return output, prev_state

        
        #return self.layer_2(self.activation_1(self.layer_1(x))), "ignored"

    #-------------------------------------------------------------------------------------------
    def get_initial_state(self, examples_per_batch) :
        #(cell state vector, hidden (output) state vector)
        # first argurment is num_layers
        # For LSTM
        return (torch.zeros(self.num_lstm_layers, examples_per_batch, self.lstm_hidden_size),
                torch.zeros(self.num_lstm_layers, examples_per_batch, self.lstm_hidden_size))

        # For LSTMCell
        #return (torch.zeros(sequence_length, self.lstm_hidden_size),
        #        torch.zeros(sequence_length, self.lstm_hidden_size))
        
        # For RNN
        # return torch.zeros(self.num_lstm_layers, examples_per_batch, self.lstm_hidden_size)

    #-------------------------------------------------------------------------------------------
    #https://closeheat.com/blog/pytorch-lstm-text-generation-tutorial
    def do_forward_batch_and_get_loss(self, examples_per_batch, state, is_training):
        if is_training is True:
            self.train()
        else:
            self.eval()
        
        loss_function = nn.CrossEntropyLoss()
        #loss_function = torch.nn.MSELoss()
        #loss_function = torch.nn.BCELoss()
        #loss_function = torch.nn.NLLLoss()
        input, target_output = self.get_random_training_batch(examples_per_batch, is_training)
        #input, target_output = self.get_random_training_sequence(is_training)
        
        input = torch.FloatTensor(input)
        #target_output = torch.FloatTensor(target_output)
        target_output = torch.LongTensor(target_output) #For CrossEntropyLoss  Loss
        
        
        if self.use_cpu == False:
            input  = input.cuda()
            target_output = target_output.cuda()

        output, state = self(input, state)
        
        
        loss = 0;
        for i in range(len(output)):
          loss += loss_function(output[i], target_output[i])
          
        return loss, state
    
    #-------------------------------------------------------------------------------------------
    #https://www.kdnuggets.com/2020/07/pytorch-lstm-text-generation-tutorial.html
    def train_model(self, num_batches, examples_per_batch, save_every, lr) :
        optimizer = optim.Adam(self.parameters())
        #optimizer = optim.SGD(self.parameters(), lr, momentum=0.9)
        for p in optimizer.param_groups : p['lr'] = lr
        start = time.time()
        
        #todo: how often to init state? Tutorial has once per epoch...
        #loss_function = torch.nn.BCELoss()
        
        for batch in range(self.get_saved_num_batches(), num_batches) :
            optimizer.zero_grad()
            state = self.get_initial_state(examples_per_batch)
            
            loss, state = self.do_forward_batch_and_get_loss(examples_per_batch, state, True)
            loss.backward()
            #torch.nn.utils.clip_grad_norm_(self.parameters(), 1)
            optimizer.step()
            
            elapsed = self.time_since(start)
            speed = (time.time() - start) / (batch + 1)

            print("Batch {0} of {1} --- Training Loss: {2} --- Elapsed Time: {3} --- Sec / Batch: {4}".format(batch + 1, num_batches, loss.item(), elapsed, speed))
            if (((batch+1) % save_every) == 0) or (batch is num_batches-1):
                self.save_checkpoint(batch+1)
                self.sample();


    #-------------------------------------------------------------------------------------------
    def sample(self):
        #self.reverse_synthesize_gold_standard("Bach_Minuet")
        #self.reverse_synthesize_gold_standard("Fugue")
        #self.reverse_synthesize_gold_standard("Bach_Saraband")
        #self.reverse_synthesize_gold_standard("MIDI_1")
        #self.reverse_synthesize_gold_standard("MIDI_2")
        self.reverse_synthesize_gold_standard("Bass")
        self.reverse_synthesize_gold_standard("Soprano")
        self.reverse_synthesize_gold_standard("All_But_Alto")
        self.reverse_synthesize_gold_standard("Alto_Tenor")
        #state = self.get_initial_state(1);
        #input = np.random.rand(self.input_size + self.output_size + 1)
       
        
        #num_tests = 10000;
        
        #start_time = time.time()
        #for i in range(num_tests):
        #  output, state = self.forward(input, state);
        #end_time = time.time()

        #print("{} iterations in {} seconds".format(num_tests, end_time-start_time))
        #in_arr = input.data.numpy()[0][0]
        #print("matrix_val_t input[] = {");
        #for i in range(len(in_arr)):
          #print(in_arr[i], end=", ");
        #print("};");
        
        #out_arr = output.data.numpy()[0][0]
        #print("matrix_val_t output[] = {");
        #for i in range(len(out_arr)):
        #  print(out_arr[i], end=", ");
        #print("};");
        #print(input.data.numpy());
        #print(output.data.numpy());
        

    #-------------------------------------------------------------------------------------------
    def export(self):
        folder = os.path.join(self.session_directory, "matrices")
        if not os.path.exists(folder):
          os.makedirs(folder)
        
        layer_1_weights = self.layer_1.weight.data.numpy()
        np.save(os.path.join(folder, "layer_1_weights.npy"), layer_1_weights)
        
        layer_1_biases = self.layer_1.bias.data.numpy();
        np.save(os.path.join(folder, "layer_1_biases.npy"), layer_1_biases)

        #(W_ii|W_if|W_ig|W_io)
        W = np.vsplit(self.lstm.weight_ih_l0.data.numpy(), 4)
        np.save(os.path.join(folder, "Wi.npy"), W[0])
        np.save(os.path.join(folder, "Wf.npy"), W[1])
        np.save(os.path.join(folder, "Wg.npy"), W[2])
        np.save(os.path.join(folder, "Wo.npy"), W[3])

        U = np.vsplit(self.lstm.weight_hh_l0.data.numpy(), 4)
        np.save(os.path.join(folder, "Ui.npy"), U[0])
        np.save(os.path.join(folder, "Uf.npy"), U[1])
        np.save(os.path.join(folder, "Ug.npy"), U[2])
        np.save(os.path.join(folder, "Uo.npy"), U[3])
        
        b = np.hsplit(self.lstm.bias_ih_l0.data.numpy() + self.lstm.bias_hh_l0.data.numpy(), 4)
        np.save(os.path.join(folder, "bi.npy"), b[0])
        np.save(os.path.join(folder, "bf.npy"), b[1])
        np.save(os.path.join(folder, "bg.npy"), b[2])
        np.save(os.path.join(folder, "bo.npy"), b[3])

        layer_2_weights = self.layer_2.weight.data.numpy()
        np.save(os.path.join(folder, "layer_3_weights.npy"), layer_2_weights)
        
        layer_2_biases = self.layer_2.bias.data.numpy()
        np.save(os.path.join(folder, "layer_3_biases.npy"), layer_2_biases)
  
    #-------------------------------------------------------------------------------------------
    def sample_softmax_with_temperature(self, x, temperature) :
        return x.argmax().item()
        #x = x / temperature
        #p = torch.nn.functional.softmax(x, dim=0).detach().numpy()
        #return np.random.choice(len(x), p=p)
        
    #-------------------------------------------------------------------------------------------
    def reverse_synthesize_gold_standard(self, filename) :
        gold_standards = glob.glob(os.path.join(self.data_directory, "Validation/Gold_Standard/", filename + ".wav"))
        
        if len(gold_standards) < 1:
            print("Unable to find file {} for reverse synthesis".format(filename))
            return
        gold_standard = gold_standards[0]
        wav, sr = librosa.load(gold_standard, sr=self.sample_rate, mono=True)
        midi = MidiFile()
        track = MidiTrack()
        midi.tracks.append(track)
        start_sample = 0
        prev_notes = []
        output = torch.zeros(self.output_size) #np.zeros(self.output_size)
        #smoothed_spectrum = np.zeros(self.input_size)
        #smoothing_coefficient = 0.99
        frames_since_last_event = 0
        
        on_for = 1.0
        off_for = 1.0
        on_count = np.zeros(self.output_size)
        
        self.eval();
        
        state = self.get_initial_state(1)
        
        counter = 0;
        prev_active_note = -1;
        
        while (start_sample + self.input_size) < len(wav):
        
            input = wav[start_sample : start_sample + self.input_size]
            
            #input = [[self.calculate_audio_features(input)]]
            #input = [self.calculate_audio_features(input)]
            
            start_time = time.time()
            input = self.calculate_audio_features(input)
            
            #autoregressive input
            prev_output_vector, active_note = self.output_notes_to_vector(prev_notes)
            
            counter = counter + 1
            if(active_note != prev_active_note):
              counter = 0;
            prev_active_note = active_note
            
            input = [[np.concatenate((input, prev_output_vector, [counter]))]]
            
            #input = [np.concatenate((input, output.detach().numpy()))]
            
            #input = np.multiply(input, 2);
            
            #input = np.multiply(input, 1.0-smoothing_coefficient);
            #smoothed_spectrum = np.multiply(input, smoothing_coefficient);
            #input = np.add(input, smoothed_spectrum)
            
            current_notes = []
            
            input = torch.FloatTensor(input)
            
            output, state = self(input, state)
                        
            output = output[0][0]; #remove superfluous dimensions
            
            end_time = time.time()
            #print(1000000 * (end_time - start_time))
            
            #output = output * 100000;
            #print(output)

            #max_output_index = output.argmax().item();
            #not really the max, just a sample with temperature
            temperature = 0.5
            max_output_index = self.sample_softmax_with_temperature(output, temperature);
            
            for i in range(len(output)) :
                #if np.random.sample() < output[i] :
                if i == max_output_index :
                    on_count[i] += 1.0 / on_for
                    if on_count[i] > 1 :
                        on_count[i] = 1
                    if (on_count[i] == 1) or (i + self.lowest_midi_note in prev_notes) :
                        current_notes.append(i + self.lowest_midi_note)
            
                else :
                    on_count[i] -= 1.0 / off_for
                    if on_count[i] <= 0 :
                        on_count[i] = 0
                    elif (i + self.lowest_midi_note in prev_notes) :
                        current_notes.append(i + self.lowest_midi_note)
            
            
            #for i in range(len(output)) :
            #    if output[i] > 0.5 :
            #    if np.random.sample() < output[i] :
            #        current_notes.append(i + self.lowest_midi_note)

            note_ons = np.setdiff1d(current_notes, prev_notes, assume_unique=True)
            note_offs = np.setdiff1d(prev_notes, current_notes, assume_unique=True)

            t = round((2 * midi.ticks_per_beat * frames_since_last_event * self.hop_size / sr))
  
            for n in note_offs:
                track.append(Message('note_off', note=n, velocity=0, time=t))
                t=0

            for n in note_ons:
                track.append(Message('note_on', note=n, velocity=64, time=t))
                t=0

            if len(note_offs) + len(note_ons) == 0:
              frames_since_last_event += 1
            else:
              frames_since_last_event = 1

            prev_notes = current_notes
            start_sample += self.hop_size
            
        #turn off any remaining notes at end of file
        for n in current_notes:
          track.append(Message('note_off', note=n, velocity=0, time=t))
        
        path = os.path.join(self.session_directory, self.model_save_prefix + "gold_standard_" + filename + str(self.get_saved_num_batches()).zfill(6) + ".mid")
        midi.save(path)
