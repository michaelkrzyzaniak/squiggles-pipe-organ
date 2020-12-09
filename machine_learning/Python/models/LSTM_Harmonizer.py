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
from mido import MidiFile
from mido import MidiTrack
from mido import Message

class LSTM_Harmonizer(nn.Module) :
    #-------------------------------------------------------------------------------------------
    def __init__(self, session_directory, data_directory, use_cpu=False) :
        super(LSTM_Harmonizer, self).__init__()
        
        #inclusive of both
        self.lowest_midi_note = 36
        self.highest_midi_note = 96
        
        self.input_size = 2048
        self.hop_size = self.input_size // 2
        self.hidden_size = 64#265
        self.output_size = self.highest_midi_note - self.lowest_midi_note + 1;
        
        self.lstm = nn.LSTM(3, 3)
        
        #self.layer_1      = torch.nn.Linear(self.input_size + self.output_size, self.hidden_size);
        #self.activation_1 = torch.nn.ReLU()
        #self.activation_1 = torch.nn.Tanh()
        #self.layer_2      = torch.nn.Linear(self.hidden_size, self.output_size);
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
        for i in range(len(spectra)) :
            if abs(spectra[i]) < 0.0002 : #-74 dB
                spectra[i]= 0+0j
  
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

        for note in notes:
            while note > self.highest_midi_note:
                note -= 12
            while note < self.lowest_midi_note:
                note += 12
            #++output_array[note]
            vector[note-self.lowest_midi_note] = 1
        return vector

    #-------------------------------------------------------------------------------------------
    def get_random_training_example_from_file(self, audio_basename, is_training=True):
        training_folder = "Training" if is_training else "Validation"
        audio_path = os.path.join(self.data_directory, training_folder, "Audio", audio_basename)
        wav, sr = librosa.load(audio_path, sr=self.sample_rate, mono=True)
        length_in_samples = len(wav)
        if length_in_samples < (2*self.input_size):
            print("{} could not be loaded because it is too short.".format(audio_filename))
            return None, None
        
        start_sample = np.random.randint(self.input_size, length_in_samples - self.input_size)
        audio = wav[start_sample : start_sample+self.input_size]
        audio = self.calculate_audio_features(audio)

        midi_basename = audio_basename[:-4] + ".mid"
        midi_file = MidiFile(os.path.join(self.data_directory, training_folder, "MIDI", midi_basename))
        start_secs = start_sample / sr
        end_secs = (start_sample + self.input_size) / sr
        notes = self.get_active_MIDI_notes_in_time_range(midi_file, start_secs, end_secs)
        output_array = self.output_notes_to_vector(notes);
        
        #autoregressive_input
        prev_start_secs = (start_sample - self.hop_size) / sr
        prev_notes = self.get_active_MIDI_notes_in_time_range(midi_file, prev_start_secs, start_secs)
        prev_output_array = self.output_notes_to_vector(prev_notes);
        audio = np.concatenate((audio, prev_output_array))
        
        return audio, output_array

    #-------------------------------------------------------------------------------------------
    def get_random_training_batch(self, examples_per_batch, is_training=True):
        input_data = [];
        output_data = [];
        training_folder = "Training" if is_training else "Validation"
        wav_paths = glob.glob(os.path.join(self.data_directory, training_folder, "Audio/*.wav"))
        
        for i in range(examples_per_batch):
            wav_index = np.random.randint(0, len(wav_paths))
            basename = os.path.basename(wav_paths[wav_index])
            x, y = self.get_random_training_example_from_file(basename, is_training)
            input_data.append(x)
            output_data.append(y)

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
    def forward(self, x) :
        hidden   = self.activation_1(self.layer_1(x))
        #hidden_2 = self.activation_1_5(self.layer_1_5(hidden))
        output   = self.activation_2(self.layer_2(hidden))
        return output

    #-------------------------------------------------------------------------------------------
    def do_forward_batch_and_get_loss(self, examples_per_batch, is_training):
        if is_training is True:
            self.train()
        else:
            self.eval()
        
        #loss_function = torch.nn.MSELoss()
        loss_function = torch.nn.BCELoss()
        input, target_output = self.get_random_training_batch(examples_per_batch, is_training)
        input = torch.FloatTensor(input)
        target_output = torch.FloatTensor(target_output)
        
        if self.use_cpu == False:
            input  = input.cuda()
            target_output = target_output.cuda()

        output = self(input)
        return loss_function(output, target_output)
    
    #-------------------------------------------------------------------------------------------
    def train_model(self, num_batches, examples_per_batch, save_every, lr) :
        optimizer = optim.Adam(self.parameters())
        #optimizer = optim.SGD(self.parameters(), lr, momentum=0.9)
        for p in optimizer.param_groups : p['lr'] = lr
        start = time.time()
        
        for batch in range(self.get_saved_num_batches(), num_batches) :
            optimizer.zero_grad()
            loss = self.do_forward_batch_and_get_loss(examples_per_batch, True)
            loss.backward()
            #torch.nn.utils.clip_grad_norm_(self.parameters(), 1)
            optimizer.step()
            
            validation_loss = 0#self.validate(examples_per_batch)
            elapsed = self.time_since(start)
            speed = (time.time() - start) / (batch + 1)

            print("Batch {0} of {1} --- Training Loss: {2} --- Validation Loss: {3} --- Elapsed Time: {4} --- Sec / Batch: {5}".format(batch + 1, num_batches, loss.item(), validation_loss, elapsed, speed))
            if (((batch+1) % save_every) == 0) or (batch is num_batches-1):
                self.save_checkpoint(batch+1)
                self.reverse_synthesize_gold_standard("Josquin")
                self.reverse_synthesize_gold_standard("Fugue")
                self.reverse_synthesize_gold_standard("Flute")
                self.reverse_synthesize_gold_standard("MIDI")


    #-------------------------------------------------------------------------------------------
    def validate(self, examples_per_batch) :
        loss = self.do_forward_batch_and_get_loss(examples_per_batch, False)
        return loss.item()

    #-------------------------------------------------------------------------------------------
    def sample(self):
        self.reverse_synthesize_gold_standard("Josquin")
        self.reverse_synthesize_gold_standard("Fugue")
        self.reverse_synthesize_gold_standard("Flute")
        self.reverse_synthesize_gold_standard("MIDI")
    
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
        
        on_for = 4.0
        off_for = 4.0
        on_count = np.zeros(self.output_size)
        
        while (start_sample + self.input_size) < len(wav):
            input = wav[start_sample : start_sample + self.input_size]
            input = self.calculate_audio_features(input)
            prev_output_vector = self.output_notes_to_vector(prev_notes)
            input = np.concatenate((input, prev_output_vector))
            #input = np.concatenate((input, output.detach().numpy()))
            
            #input = np.multiply(input, 1.0-smoothing_coefficient);
            #smoothed_spectrum = np.multiply(input, smoothing_coefficient);
            #input = np.add(input, smoothed_spectrum)
            
            current_notes = []
            
            input = torch.FloatTensor(input)
            output = self(input)
            for i in range(len(output)) :
                if np.random.sample() < output[i] :
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
        path = os.path.join(self.session_directory, self.model_save_prefix + "gold_standard_" + filename + str(self.get_saved_num_batches()).zfill(6) + ".mid")
        midi.save(path)
