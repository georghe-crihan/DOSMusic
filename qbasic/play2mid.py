from struct import pack

def write_var_len(v: int) -> str:
    # a: str
    a=chr(v and 127)

    while v > 127:
        v = v >> 7
        a=chr((v and 127) or 128) + a
    return a

def write_four_bytes(v: int) -> str:
    return pack('<I', v)

xlatNote = { # (n: str) -> ubyte:
    "c": 0,
    "cs": 1, "db": 1,
    "d": 2,
    "ds": 3, "eb": 3,
    "e": 4, "fb" : 4,
    "f": 5, "es" : 5,
    "fs": 6, "gb": 6,
    "g": 7,
    "gs": 8, "ab" 8,
    "a": 9,
    "as": 10, "bb": 10,
    "b": 11, "cb": 11
    }


def _fbplay_internal(channel: ubyte, playstr: str) -> str:
    """
	default tempo is 120 quarter notes per minute
	default note is a quarter note
	as default notes play their full length
	default octave is the 4th
	default instrument is acoustic grand piano |TODO: Find a instrument closer to QB's PLAY sound.
	maximum volume is default
	"""

    # Track: str

	tempo: uint = 120
    note_len: ubyte = 4
    note_len_mod:  double = 1
    octave: ubyte = 4
    volume:  ubyte = 127
    note_stack[128]: ubyte

    #chord: ubyte
    #next_event: double

    #duration: double
    #idx: ubyte
       
    #number: str
    #ch:  char
	#tChar: char
       
	#toTranslate: str

	p = 1 #: int
       
    while p < len(playstr):
        ch=playstr[p].lower()
        p+=1

		# basic playing
			if ch == "n":      # plays note with next-comming number, if 0 then pause
				number=""
				while True:
					tchar=playstr[p]
					if ch.isdigit():
						p+=1
						number+=tchar
					else:
						break
				idx=int(number)

				if idx==0: # pause
					next_event+=60/tempo*(4/note_len)/60
				else: # note
					duration=60/tempo*(4/note_len)

					Track=Track+write_var_len(240*next_event)+chr(0x90 + channel)+chr(idx)+chr(volume)

					next_event=duration*(1-note_len_mod)
					#stop_note(channel)=t+duration*note_len_mod(channel)

					note_stack[0]+=1
					note_stack[note_stack[0]]=idx
		       
		   
			elif ch >= "a" and ch <= "g":      # plays a to g in current octave         
				duration=60/tempo*(4/note_len)
		         
				toTranslate=ch

				number=""
				ch=playstr[p]
				if ch=="-":
					toTranslate+="b"
					p+=1
				elif ch=="+" or ch=="#":
					toTranslate+="s"
					p+=1


				while True:
					ch=playstr[p]
					if ch.isdigit():
						p+=1
						number+=ch
					else:
						break
				if int(number)!=0: duration=duration*4/int(number)
				if ch==".": duration=duration*1.5

				idx=12*octave+xlatNote[toTranslate]

				Track=Track+write_var_len(240*next_event)+chr(0x90 + channel)+chr(idx)+chr(volume)

				next_event=duration*(1-note_len_mod)

				note_stack[0]+=1
				note_stack[note_stack[0]]=idx


			elif ch == "p":      # pauses for next-comming number of quarter notes
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit():
						p+=1
						number+=ch
					else:
						break
				next_event+=60/tempo*4/int(number)
		         
		   
			# octave handling
			elif ch == ">":      # up one octave
				if octave<7: octave+=1
		         
			elif ch == "<":      # down one octave
				if octave>1: octave-=1
		         
			elif ch == "o":      # changes octave to next-comming number
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit(): 
						p+=1
						number+=ch
					else:
						break
				octave=int(number)
		         
		         
			# play control
			elif ch == "t":      # changes tempo (quarter notes per minute)
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit():
						p+=1
						number+=ch
					else:
						break
				tempo=int(number)

			elif ch == "l":      # changes note length (1=full note, 4=quarter note, 8 eigth(?) note aso)
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit(): 
						p+=1
						number+=ch
					else:
                                            break
				note_len=int(number)
		     
			elif ch == "m":      # MS makes note last 3/4, MN is 7/8 and ML sets to normal length
				ch=playstr[p].lower()
				p+=1
				if ch=="s": note_len_mod=3/4
				if ch=="n": note_len_mod=7/8
				if ch=="l": note_len_mod=1
		     
		     
			# new MIDI fucntions
			elif ch == "i":
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit():
						p+=1
						number+=ch
					else:
						break
				Track=Track+write_var_len(0)+chr(0xc0 + channel)+chr(int(number))
		     
			elif ch == "v":
				number=""
				while True:
					ch=playstr[p]
					if ch.isdigit():
						p+=1
						number+=ch
					else:
						break
				volume=int(number)
			elif ch == "{":      # enable chords (notes play simultaneously)
				chord=1
			Case "}"      # disable chords (notes play simultaneously)
				chord=0


		if chord: 
			if chord==2: next_event=0 else: chord=2
		else:
			# Stop current note, if still playing
			for i in range(1, note_stack[0]):
				Track=Track+write_var_len(240*duration*note_len_mod)+chr(0x80 + channel)+chr(note_stack[i])+chr(0)
				duration=0
			note_stack[0]=0


	return Track
       

def play(playstr: str, playstr1: str="", playstr2: str="", playstr3: str="",
	playstr4: str="", playstr5: str="", playstr6: str="", playstr7: str="",
	playstr8: str="", playstr9: str="", playstr10: str="", playstr11: str="",
	playstr12: str="", playstr13: str="", playstr14: str="", playstr15: str="")

	#if _fbplay_internal_playstr(0)[:2].lower()=="mb":     # supposed to play in foreground 

	Tracks: int

	midi: str
	Track: str
	Track=_fbplay_internal(0,playstr)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(1,playstr1)
	if len(Track)>0: 
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(2,playstr2)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(3,playstr3)
	if len(Track)>0: 
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(4,playstr4)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(5,playstr5)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(6,playstr6)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(7,playstr7)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(8,playstr8)
	if len(Track)>0: 
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(9,playstr9)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(10,playstr10)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(11,playstr11)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(12,playstr12)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(13,playstr13)
	if len(Track)>0:
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(14,playstr14)
	if len(Track)>0: 
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	Track=_fbplay_internal(15,playstr15)
	if len(Track)>0: 
		Midi=Midi +"MTrk"+write_four_bytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1

	with open("output.mid", "wa") as output: 
	    output.write("MThd"+chr(0)+chr(0)+chr(0)+chr(6)+chr(0)+chr(1 if Tracks > 1 else 0))+chr(0)+chr(Tracks)+chr(0)+chr(120)+Midi)


play(" i48 t200l4mneel2el4eel2el4egl3cl8dl1el4ffl3fl8fl4fel2el8eel4eddel2dgl4eel2el4eel2el4egl3cl8dl1el4ffl3fl8fl4fel2el8efl4ggfdl2cl8")
