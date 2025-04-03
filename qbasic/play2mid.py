#!/usr/bin/env python3
from struct import pack


def write_var_len(v: int) -> bytes:
    # a: str
    a = [v and 127]

    while v > 127:
        v = v >> 7
        a.insert(0, (v and 127) or 128)
    return bytes(a)


def write_four_bytes(v: int) -> bytes:
    # a: str
    a = [v and 255]
    v >>= 8
    a.insert(0, v and 255)
    v >>= 8
    a.insert(0, v and 255)
    v >>= 8
    a.insert(0, v and 255)
    return bytes(a)
#    return pack('<I', v)


xlatNote = {  # (n: str) -> ubyte:
    "c": 0,
    "cs": 1, "db": 1,
    "d": 2,
    "ds": 3, "eb": 3,
    "e": 4, "fb": 4,
    "f": 5, "es": 5,
    "fs": 6, "gb": 6,
    "g": 7,
    "gs": 8, "ab": 8,
    "a": 9,
    "as": 10, "bb": 10,
    "b": 11, "cb": 11
    }


def _fbplay_internal(channel: int, playstr: str) -> bytes:
    """
    channel: ubyte, playstr: str

    default tempo is 120 quarter notes per minute
    default note is a quarter note
    as default notes play their full length
    default octave is the 4th
    default instrument is acoustic grand piano
          |TODO: Find a instrument closer to QB's PLAY sound.
    maximum volume is default
    """

    track = b""  # : bytes

    tempo = 120  # : uint
    note_len = 4  # : ubyte
    note_len_mod = 1  # : double
    octave = 4  # : ubyte
    volume = 127  # : ubyte
    note_stack = []  # : ubyte

    chord = 0  # : ubyte
    next_event = 0  # : double

    duration = 0  # : double
    # idx: ubyte

    # number: str
    # ch: char
    # tChar: char

    # toTranslate: str

    p = 0  # : int

    while p < len(playstr):
        ch = playstr[p].lower()
        p += 1

        # basic playing
        if ch == "n":  # plays note with next-coming number, if 0 then pause
            number = ""
            while p < len(playstr):
                tchar = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += tchar
                else:
                    break
            idx = int(number)

            if idx == 0:  # pause
                next_event += 60/tempo*(4/note_len)/60
            else:  # note
                duration = 60/tempo*(4/note_len)

            track = (track + write_var_len(240*next_event) +
                     bytes([0x90 + channel, idx, volume]))

            next_event = duration*(1-note_len_mod)
            # stop_note(channel) = t+duration*note_len_mod(channel)

            note_stack.append(idx)

        elif "a" <= ch <= "g":  # plays a to g in current octave
            duration = 60/tempo*(4/note_len)

            to_translate = ch

            ch = playstr[p]
            if ch == "-":
                to_translate += "b"
                p += 1
            elif ch == "+" or ch == "#":
                to_translate += "s"
                p += 1

            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            if number != "" and int(number) != 0:
                duration = duration*4/int(number)
            if ch == ".":
                duration = duration*1.5

            idx = 12*octave+xlatNote[to_translate]

            track = (track + write_var_len(round(240*next_event)) +
                     bytes([0x90 + channel, idx, volume]))

            next_event = duration * (1 - note_len_mod)

            note_stack.append(idx)

        elif ch == "p":  # pauses for next-coming number of quarter notes
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            next_event += 60/tempo*4/int(number)

        # octave handling
        elif ch == ">":      # up one octave
            if octave < 7:
                octave += 1

        elif ch == "<":      # down one octave
            if octave > 1:
                octave -= 1

        elif ch == "o":      # changes octave to next-comming number
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            octave = int(number)

        # play control
        elif ch == "t":      # changes tempo (quarter notes per minute)
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            tempo = int(number)

        elif ch == "l":  # changes note length (1=full note, 4=quarter note, 8 eigth(?) note aso)
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            note_len = int(number)

        elif ch == "m":  # MS makes note last 3/4, MN is 7/8 and ML sets to normal length
            ch = playstr[p].lower()
            p += 1
            if ch == "s":
                note_len_mod = 3/4
            if ch == "n":
                note_len_mod = 7/8
            if ch == "l":
                note_len_mod = 1

        # new MIDI functions
        elif ch == "i":
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            track = (track + write_var_len(0) +
                     bytes([0xc0 + channel, int(number)]))

        elif ch == "v":
            number = ""
            while p < len(playstr):
                ch = playstr[p]
                if ch.isdigit():
                    p += 1
                    number += ch
                else:
                    break
            volume = int(number)
        elif ch == "{":      # enable chords (notes play simultaneously)
            chord = 1
        elif ch == "}":      # disable chords (notes play simultaneously)
            chord = 0


        if chord:
            if chord == 2:
                next_event = 0
            else:
                chord = 2
        else:
            # Stop current note, if still playing
            for i in note_stack:
                track = (track + write_var_len(round(240*duration*note_len_mod)) +
                         bytes([0x80 + channel, i, 0]))
                duration = 0
            note_stack = []

    return track


def play(playstr: str, playstr1: str = "", playstr2: str = "",
         playstr3: str = "", playstr4: str = "", playstr5: str = "",
         playstr6: str = "", playstr7: str = "", playstr8: str = "",
         playstr9: str = "", playstr10: str = "", playstr11: str = "",
         playstr12: str = "", playstr13: str = "", playstr14: str = "",
         playstr15: str = ""):

    # if _fbplay_internal(playstr)[:2].lower()=="mb":  # supposed to play in foreground

    tracks = 0  # : int

    midi = b''  # : bytes
    # track: bytes

    track = _fbplay_internal(0, playstr)
    if len(track) > 0:
        midi = (midi + b'MTrk' + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(1, playstr1)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(2, playstr2)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(3, playstr3)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(4, playstr4)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(5, playstr5)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(6, playstr6)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(7, playstr7)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(8, playstr8)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(9, playstr9)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(10, playstr10)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(11, playstr11)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(12, playstr12)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(13, playstr13)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(14, playstr14)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1
    track = _fbplay_internal(15, playstr15)
    if len(track) > 0:
        midi = (midi + b"MTrk" + write_four_bytes(len(track) + 4) +
                track + bytes([0, 255, 47, 0]))
        tracks += 1

    with open("output.mid", "ab") as output:
        output.write(b"MThd" + bytes([0, 0, 0, 6, 0, (1 if tracks > 1 else 0),
                                      0, tracks, 0, 120]) + midi
                     )


play("t130 mb ml l4 p2 o2 e. l8 d c o1 b mn a4" +
     "a ml o2 c o1 b a g a e d mn e1" +
     "o2 e4. ml d c o1 b mn a4 a ml o2 c o1 b a g a e d e1" +
     "o2 mn d d d ml c o1 mn f4 f ml g o2 e4 d c o1 f4 a o2 c" +
     "o1 b4 a mn g g4 ml a g a1"
     )
