These are the Objective-C classes needed to read and write MIDI FIles. In general, MKMIDIEvent and MKMIDIEventList should not need to be used directly. MKMIDIFile should be used, and it will invoke MKMIDIEvent and MKMIDIEventList when necessary. MKArray is a good multi-purpose linked list that may be used for anything. Opus49.m is a simple demo program that scribbles a short MIDI file into the directory from which it was invoked (probably your home directory if you double clicked it's binary).  

Although this code works great on my computer, it will probably crash yours beyond repair, so you should probably not use it for anything at all.

MK 8/27/10