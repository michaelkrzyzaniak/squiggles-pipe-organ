
/*----------------------------------------------------------------------
         _          __________                              _,
     _.-(_)._     ."          ".      .--""--.          _.-{__}-._
   .'________'.   | .--------. |    .'        '.      .:-'`____`'-:.
  [____________] /` |________| `\  /   .'``'.   \    /_.-"`_  _`"-._\
  /  / .\/. \  \|  / / .\/. \ \  ||  .'/.\/.\'.  |  /`   / .\/. \   `\
  |  \__/\__/  |\_/  \__/\__/  \_/|  : |_/\_| ;  |  |    \__/\__/    |
  \            /  \            /   \ '.\    /.' / .-\                /-.
  /'._  --  _.'\  /'._  --  _.'\   /'. `'--'` .'\/   '._-.__--__.-_.'   \
 /_   `""""`   _\/_   `""""`   _\ /_  `-./\.-'  _\'.    `""""""""`    .'`\
(__/    '|    \ _)_|           |_)_/            \__)|        '       |   |
  |_____'|_____|   \__________/   |              |;`_________'________`;-'
   '----------'    '----------'   '--------------'`--------------------`
------------------------------------------------------------------------
   _____                _
  |_   _|__ _ _  ___   (_)___
    | |/ _ \ ' \/ -_)_ | (_-<
    |_|\___/_||_\___(_)/ /__/
                     |__/
------------------------------------------------------------------------

  Tone generator
  For Ambisynth project
  Made by Michael Krzyzaniak
  michael.krzyzaniak@yahoo.com
 
  The public interface of this code is intended to be:
 
 
  Version:
    1.0

----------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
Tone.audio_context      = null;
Tone.MIN_GAIN           = -69;

if('webkitAudioContext' in window)
  Tone.audio_context = new webkitAudioContext();
else if('AudioContext' in window)
  Tone.audio_context = new AudioContext();

/*--------------------------------------------------------------------*/
function Tone()
{
  this.corpuses = [];
  this.current_corpus_name = "";
 
  this.gain = 1;
  this.num_channels = 1;
  this.freq = 0;
  this.phase = 0;
  
  this.set_freq(440);
 
  this.was_playing    = false;
  this.is_playing     = false;
  
  this.script_node = Tone.audio_context.createScriptProcessor(4096, 0, this.num_channels);
  this.script_node.onaudioprocess = this.onaudioprocess.bind(this);
}


/*--------------------------------------------------------------------*/
Tone.prototype.destroy = function()
{

};

/*--------------------------------------------------------------------*/
Tone.prototype.set_freq = function(cps)
{
  this.freq = cps * 2 * Math.PI / Tone.audio_context.sampleRate;
};


/*--------------------------------------------------------------------*/
Tone.CONSTRAIN = function(input, min, max)
{
  return (input > max) ? max : (input < min) ? min : input;
};

/*--------------------------------------------------------------------*/
Tone.prototype.get_gain = function()
{
  return 20 * Math.log10(this.gain);
};

/*--------------------------------------------------------------------*/
//gains less than -69 dB will be treated as -inf
Tone.prototype.set_gain = function(db)
{
  if(db < Tone.MIN_GAIN)
    this.gain = 0
  else
    this.gain = Math.pow(10, (db / 20.0));
};

/*--------------------------------------------------------------------*/
Tone.prototype.play = function()
{
  Tone.audio_context.resume();
  Tone.unlock_web_audio_if_necessary();
  if(!this.is_playing)
    {
      this.script_node.connect(Tone.audio_context.destination);
      this.is_playing = true;
    }
};

/*--------------------------------------------------------------------*/
Tone.prototype.pause = function()
{
  if(this.is_playing)
    {
      this.script_node.disconnect(Tone.audio_context.destination);
      this.is_playing = false;
    }
};


/*--------------------------------------------------------------------*/
Tone.prototype.onaudioprocess = function(audio_processing_event)
{
  var chan, frame;
  var num_channels = audio_processing_event.outputBuffer.numberOfChannels;
  var buffer = audio_processing_event.outputBuffer;
  var num_frames = audio_processing_event.outputBuffer.getChannelData(0).length;

  //annoyingly, webAudio wants de-interleaved data, so we have to manually de-interleave it
  //even though the kernel will eventually have to re-interleave it.
  //lets do it in chunks to save space.
  var intermediate_buffer_num_frames = 256;
  var total_frames_written = 0;
  var buffer = new Array(intermediate_buffer_num_frames * num_channels);
  while(total_frames_written < num_frames)
    {
      var start_frame = total_frames_written;
      var requested_frames = num_frames - total_frames_written;
      requested_frames = Tone.CONSTRAIN(requested_frames, 1, intermediate_buffer_num_frames);
      total_frames_written += this.audio_callback(buffer, requested_frames, num_channels);
    
      for(chan=0; chan<num_channels; chan++)
        {
          var b = audio_processing_event.outputBuffer.getChannelData(chan);
          for(frame=0; frame<requested_frames; frame++)
            b[frame + start_frame] = buffer[frame*num_channels+chan];
        }
    }
};

/*--------------------------------------------------------------------*/
Tone.prototype.audio_callback = function(buffer, num_frames, num_channels)
{
  var i, j, k=0;
  for(i=0; i<num_frames; i++)
    for(j=0; j<num_channels; j++)
      {
        buffer[k++] = Math.sin(this.phase);
        this.phase += this.freq;
        if(this.phase > (2*Math.PI))
          this.phase -= (2*Math.PI);
      }
  return num_frames;
};

/*--------------------------------------------------------------------*/
// Evidently, on ios, we can not play sounds at arbitrary times unless
// the user presses a button to trigger a sound at least once.
// So this method plays a silent audio file. This should
// be called in response to some random button press. Doing this
// will then allow the app to speak at arbitrary times.
Tone.webaudio_is_unlocked = false;
Tone.unlock_web_audio_if_necessary = function()
{
  var is_ios = !!navigator.platform && /iPad|iPhone|iPod/.test(navigator.platform);
  if((is_ios === true) && (Tone.webaudio_is_unlocked === false))
    {
      var buffer = Tone.audio_context.createBuffer(1, 1, 22050);
      var source = Tone.audio_context.createBufferSource();
      source.buffer = buffer;
      source.connect(Tone.audio_context.destination);
      source.start();

      // playbackState is deprecated but still used by ios at the moment
      setTimeout(function()
      {
        if((source.playbackState === source.PLAYING_STATE) || (source.playbackState === source.FINISHED_STATE))
          Tone.webaudio_is_unlocked = true;
      }, 100);
    }
};


