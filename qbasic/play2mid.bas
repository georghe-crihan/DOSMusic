FUNCTION WriteVarLen(Value as integer) as string
 dim a as string

 a=chr(Value AND 127)

  DO WHILE (Value > 127)
   Value = Value shr 7
   a=chr((Value AND 127)or 128)+a
  LOOP 
 return a
END FUNCTION

FUNCTION WriteFourBytes(Value as integer) as string
 dim a as string
 a=chr(Value and 255)
 Value shr= 8
 a=chr(Value and 255)+a
 Value shr= 8
 a=chr(Value and 255)+a
 Value shr= 8
 a=chr(Value and 255)+a
 return a
end function




function _fbplay_internal_translateNote(toTranslate as string) as ubyte
	select case toTranslate
	case "c"  : return 0

	case "cs" : return 1
	case "db" : return 1

	case "d"  : return 2

	case "ds" : return 3
	case "eb" : return 3

	case "e"  : return 4
	case "fb" : return 4

	case "f"  : return 5
	case "es" : return 5

	case "fs" : return 6
	case "gb" : return 6

	case "g"  : return 7

	case "gs" : return 8
	case "ab" : return 8

	case "a"  : return 9

	case "as" : return 10
	case "bb" : return 10

	case "b"  : return 11
	case "cb" : return 11
	end select
       
end function



function _fbplay_internal(channel as ubyte, playstr as string) as string
       
	'default tempo is 120 quarter notes per minute
	'default note is a quarter note
	'as default notes play their full length
	'default octave is the 4th
	'default instrument is acoustic grand piano |TODO: Find a instrument closer to QB's PLAY sound.
	'maximum volume is default

	dim Track as string

	dim tempo as uinteger = 120
	dim note_len as ubyte = 4
	dim note_len_mod as double = 1
	dim octave as ubyte = 4
	dim volume as ubyte = 127
	dim note_stack(128) as ubyte

	dim chord as ubyte
	dim next_event as double

       
       
	dim duration as double
	dim idx as ubyte
       
	dim number as string
	dim char as string*1
	dim tChar as string*1
       
	dim toTranslate as string

	dim p as integer=1
       
	do while p < len(playstr)

		char=lcase(mid(playstr, p, 1))
		p+=1

		select case char
		 
			'basic playing
			case "n"      'plays note with next-comming number, if 0 then pause
				number=""
				do
					tchar=mid(playstr, p, 1)
					if asc(tchar)>=48 and asc(tchar)<=57 then
						p+=1
						number+=tchar
					else
						exit do
					end if
				loop
				idx=val(number)

				if idx=0 then 'pause
					next_event+=60/tempo*(4/note_len)/60
				else 'note
					duration=60/tempo*(4/note_len)

					Track=Track+WriteVarLen(240*next_event)+chr(&H90 + channel)+chr(idx)+chr(volume)

					next_event=duration*(1-note_len_mod)
					'stop_note(channel)=t+duration*note_len_mod(channel)

					note_stack(0)+=1
					note_stack(note_stack(0))=idx
				end if
		       
		   
			case "a" to "g"      'plays a to g in current octave         
				duration=60/tempo*(4/note_len)
		         
				toTranslate=char

				number=""
				char=mid(playstr, p, 1)
				if char="-" then
					toTranslate+="b"
					p+=1
				elseif char="+" or char="#" then
					toTranslate+="s"
					p+=1
				end if

				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				if val(number)<>0 then duration=duration*4/val(number)
				if char="." then duration=duration*1.5

				idx=12*octave+_fbplay_internal_translateNote(toTranslate)

				Track=Track+WriteVarLen(240*next_event)+chr(&H90 + channel)+chr(idx)+chr(volume)

				next_event=duration*(1-note_len_mod)

				note_stack(0)+=1
				note_stack(note_stack(0))=idx


			case "p"      'pauses for next-comming number of quarter notes
				number=""
				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				next_event+=60/tempo*4/val(number)
		         
		   
			'octave handling
			case ">"      'up one octave
				if octave<7 then octave+=1
		         
			case "<"      'down one octave
				if octave>1 then octave-=1
		         
			case "o"      'changes octave to next-comming number
				number=""
				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				octave=val(number)
		         
		         
			'play control
			case "t"      'changes tempo (quarter notes per minute)
				number=""
				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				tempo=val(number)

			case "l"      'changes note length (1=full note, 4=quarter note, 8 eigth(?) note aso)
				number=""
				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				note_len=val(number)
		     
			case "m"      'MS makes note last 3/4, MN is 7/8 and ML sets to normal length
				char=lcase(mid(playstr, p, 1))
				p+=1
				if char="s" then note_len_mod=3/4
				if char="n" then note_len_mod=7/8
				if char="l" then note_len_mod=1
		     
		     
			'new midi fucntions
			case "i"
				number=""
				do
		           
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				Track=Track+WriteVarLen(0)+chr(&HC0 + channel)+chr(val(number))
		     
			case "v"
				number=""
				do
					char=mid(playstr, p, 1)
					if asc(char)>=48 and asc(char)<=57 then
						p+=1
						number+=char
					else
						exit do
					end if
				loop
				volume=val(number)
			Case "{"      'enable chords (notes play simultaneously)
				chord=1
			Case "}"      'disable chords (notes play simultaneously)
				chord=0

			case else
		end select


		if chord then 
			if chord=2 then next_event=0 else chord=2
		else
			'Stop current note, if still playing
			for i as integer=1 to note_stack(0)
				Track=Track+WriteVarLen(240*duration*note_len_mod)+chr(&H80 + channel)+chr(note_stack(i))+chr(0)
				duration=0
			next
			note_stack(0)=0
		end if

	loop

	return Track
       
end function
  


sub play (playstr as string, playstr1 as string="", playstr2 as string="", playstr3 as string="", _
	playstr4 as string="", playstr5 as string="", playstr6 as string="", playstr7 as string="", _
	playstr8 as string="", playstr9 as string="", playstr10 as string="", playstr11 as string="", _
	playstr12 as string="", playstr13 as string="", playstr14 as string="", playstr15 as string="")

	'if lcase(left(_fbplay_internal_playstr(0),2))="mb" then    'supposed to play in foreground 

	dim Tracks as integer

	dim midi as string
	dim Track as string
	Track=_fbplay_internal (0,playstr)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (1,playstr1)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (2,playstr2)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (3,playstr3)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (4,playstr4)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (5,playstr5)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (6,playstr6)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (7,playstr7)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (8,playstr8)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (9,playstr9)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (10,playstr10)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (11,playstr11)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (12,playstr12)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (13,playstr13)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (14,playstr14)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if
	Track=_fbplay_internal (15,playstr15)
	if len(Track)>0 then
		Midi=Midi +"MTrk"+WriteFourBytes(len(Track)+4)+Track+chr(0)+chr(255)+chr(47)+chr(0)
		Tracks+=1
	end if

	open "output.mid" for output as #2
	?#2,"MThd"+chr(0)+chr(0)+chr(0)+chr(6)+chr(0)+chr(iif(Tracks>1,1,0))+chr(0)+chr(Tracks)+chr(0)+chr(120)+Midi;
	close    
end sub 


PLAY " i48 t200l4mneel2el4eel2el4egl3cl8dl1el4ffl3fl8fl4fel2el8eel4eddel2dgl4eel2el4eel2el4egl3cl8dl1el4ffl3fl8fl4fel2el8efl4ggfdl2cl8"