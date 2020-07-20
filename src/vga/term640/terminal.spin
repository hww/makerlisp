''
'' VGA terminal, Luther Johnson
''
'' Uses (modified) Parallax "VGA Hires Text Driver" v1.0 (Chip Gracey),
'' and (unmodified) "Full Duplex COM Engine" v2.0 (Kwabena W. Agyeman)
'' and "Basic I2C Routines" (Michael Green).
''
'' Inspired and informed by the Hobbytronics "Serial VGA", the Maccasoft
'' "RC2014 VGA Terminal" (Marco Maccaferri), and the "waitvid" family of
'' VGA drivers (Marko Lukat).

CON

  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000

  ' Screen geometry from driver
  cols = vgatext#cols
  rows = vgatext#rows

  ' VGA base pin
  VGAB = 16
  
  ' I2C EEPROM characteristics
  I2CSCL = 28
  EEPROM = i2c#EEPROM

  ' Serial port characteristics
  RX   = 10
  TX   = 9
  BAUD = 115200

  ' READY pin, LED
  LED  = 25
  RDY  = 26

  ' Control and special characters
  NUL  = 0
  ENQ  = 5
  BEL  = 7
  BS   = 8
  NL   = 10
  CR   = 13
  XON  = (17 + 128)
  XOFF = (19 + 128)
  ESC  = 27
  SP   = 32

  ' Beep speaker to sound the bell               
  BELL_PINA = 14
  BELL_PINB = 15
  BELL_FREQ = 440
  BELL_MS   = 250
  SCLK_FREQ = 80_000_000

  ' Default foreground/background colors
  DEFCOLORS = %%000_0_321_0

OBJ

  serial  : "Full-Duplex_COMEngine"
  vgatext : "VGA_HiRes_Text"
  i2c     : "BasicI2C"

VAR

  ' For VGA text driver
  ' sync - written to -1 by VGA driver after each screen refresh
  ' screen buffer - longs allow more efficient scrolling
  ' row colors
  ' cursor control bytes
  long sync
  long screen[(cols*rows*2)/4]
  word colors[rows]
  byte cx0, cy0, cm0, cx1, cy1, cm1

  ' Current fg, bg colors, ANSI parameters, attributes
  word fbcolor
  byte fgcolor, bgcolor, reverse, bright
  byte pn, p[8]
  byte autowrap, nlmode

  ' Eeprom read/write address 
  long  romaddr

PRI Cls

  ' Set colors in all rows
  wordfill(@colors[0], fbcolor, rows)
  
  ' Clear the screen
  wordfill(@screen.word[0], (reverse << 8) | SP, rows*cols)

PRI Beep(lbeep)

  ' Beep the speaker
  frqa := trunc(53.6870912 * float(BELL_FREQ))
  ctra := (%00100 << 26) | (BELL_PINB << 9) | BELL_PINA
  waitcnt(((SCLK_FREQ / 1000) * (1 + lbeep) * BELL_MS) + cnt)
  ctra := 0

PRI Putb(c)

  ' Set fg, bg colors of row
  colors[cy0] := fbcolor

  ' Stuff character/attribute into the screen buffer at the cursor
  screen.word[cy0*cols + cx0] := (reverse << 8) | c

PRI Scroll

  ' Roll the screen up one line
  longmove(@screen.word[0], @screen.word[cols], ((rows - 1)*cols)/2)

  ' Blank the bottom line
  colors[rows - 1] := fbcolor
  wordfill(@screen.word[(rows - 1)*cols], (reverse << 8) | SP, cols)

PRI Decout(n) | b, d, z

  ' Write out a byte as an unsigned decimal number
  b := 100
  z := 1
  repeat
    d := n / b
    if (d <> 0)
      z := 0
    n := n - d*b
    if (z == 0) or (b == 1)
      serial.writeByte(d + $30)
    b := b / 10
  until (b == 0)

PRI Passcmd(c) | i

    ' Pass command to keyboard
    serial.writeByte(ESC)
    serial.writeByte("[")
    if (pn <> 0)
      i := 0
      repeat
        if (i <> 0)
          serial.writeByte(";")
        Decout(p[i])
        i := i + 1
      until (not (i < pn))
    serial.writeByte(c)

PRI Get1parm(i) | c, d, n, r

  ' Get a decimal number from input
  c := $30
  d := n := r := 0
  repeat
    r := r*10 + (c - $30)
    c := serial.readByte
    d := not ((c < $30) or ($39 < c))
    if d
      n := 1
  until not d

  p[i] := r
  pn := pn + n
  result := c

PRI Getparms | c, i

  i := 0
  pn := 0
  repeat
    c := Get1parm(i)   
    i := i + 1
  until (not (i < 8)) or (((c < $30) or ($39 < c)) and (c <> $3b))

  result := c

PRI ESCcmd | c, d, fg, bg, i

  ' Process ESC commands
  result := 0

  ' Look for "["
  c := serial.readByte
  if c <> "["
    ' Just stuff the character following ESC
    result := NEGX | c
    return

  ' Get parameters
  c := Getparms
 
  ' Decode individual commands
  case c
    "[" :
      ' Just stuff "["
      result := NEGX | c
      return
    "A" :
      ' Cursor up
      i := 1
      if (pn == 1)
        i := p[0]
      cy0 := cy0 - i
      if not (cy0 < rows)
        cy0 := 0 
    "B" :
      ' Cursor down
      i := 1
      if (pn == 1)
        i := p[0]
      cy0 := cy0 + i
      if not (cy0 < rows)
        cy0 := rows - 1 
    "C" :
      ' Cursor forward
      i := 1
      if (pn == 1)
        i := p[0]
      cx0 := cx0 + i
      if not (cx0 < cols)
        cx0 := cols - 1 
    "D" :
      ' Cursor backward
      i := 1
      if (pn == 1)
        i := p[0]
      cx0 := cx0 - i
      if not (cx0 < cols)
        cx0 := 0 
    "H" :
      ' Set cursor position
      if (pn == 2)
        if p[0] == 0
          p[0] := 1
        if not (p[0] < rows)
          p[0] := rows
        if p[1] == 0
          p[1] := 1
        if not (p[1] < cols)
          p[1] := cols
        cx0 := p[1] - 1
        cy0 := p[0] - 1
      else
        cx0 := 0
        cy0 := 0
    "J" :
      ' Clear screen
      if (pn == 1) and (p[0] == 2)
        Cls
    "R" :
      ' Read keyboard controller ROM
      Passcmd(c)
    "U" :
      ' Read VGA controller ROM
      if (pn == 2)
        romaddr := p[0] + (p[1] << 8)
      p[0] := i2c.ReadByte(I2CSCL, EEPROM, romaddr)
      romaddr := romaddr + 1
      pn := 1
      Passcmd(c)
    "V" :
      ' Write VGA controller ROM
      i := 0
      if (pn == 6)
        romaddr := p[0] + (p[1] << 8)
        i := 2
      d := p[i] + (p[i + 1] << 8) +(p[i + 2] << 16) + (p[i + 3] << 24)
      i2c.WriteLong(I2CSCL, EEPROM, romaddr, d)
      repeat
        d := i2c.WriteWait(I2CSCL, EEPROM, romaddr)
      until not d
      romaddr := romaddr + 4 
      pn := 0
      Passcmd(c)
    "W" :
      ' Write keyboard controller ROM
      Passcmd(c)
    "X" :
      ' Set foreground, background colors from ANSI palette or default
      fg := serial.readByte
      bg := serial.readByte
      if (fg == $FF) and (bg == $FF)
        fgcolor := bgcolor := 0
      else
        fgcolor := fg & $0F
        bgcolor := bg & $0F
      if (fgcolor == 0) and (bgcolor == 0)
        fbcolor := DEFCOLORS
      else
        fbcolor := (palette[bgcolor] << 8) | palette[fgcolor]
    "Y" :
      ' Set foreground, background colors directly
      fg := serial.readByte
      bg := serial.readByte
      fgcolor := bgcolor := 0
      fbcolor := (bg << 10) | fg << 2
    "Z" :
      ' Set reverse video attribute
      reverse := serial.readByte & $01
    "?" :
      c := GetParms

      ' Turn cursor on or off
      if (pn == 1) and (p[0] == 25)
        if c == "h"
          cm0 := %111
        if c == "l"
          cm0 := %000

      ' Turn autowrap on or off
      if (pn == 1) and (p[0] == 7)
        if c == "h"
          autowrap := 1
        if c == "l"
          autowrap := 0
    "h" :
      ' Set newline mode, then send on to keyboard
      if (pn == 1) and (p[0] == 20)
        nlmode := 1
        Passcmd(c)
    "l" :
      ' Reset newline mode, then send on to keyboard
      if (pn == 1) and (p[0] == 20)
        nlmode := 0
        Passcmd(c)
    "m" :
      ' Set Graphics Rendition
      i := 0
      repeat
        case p[i]
          0 :
            fgcolor := bgcolor := reverse := bright := 0
          1 :
            bright := 8               
          7 :
            reverse := 1
          10 :
            return
          11 :
            return
          22 :
            bright := 0
          27 :
            reverse := 0
          30 :
            fgcolor := 0 + bright
          31 :
            fgcolor := 1 + bright
          32 :
            fgcolor := 2 + bright
          33 :
            fgcolor := 3 + bright
          34 :
            fgcolor := 4 + bright
          35 :
            fgcolor := 5 + bright
          36 :
            fgcolor := 6 + bright
          37 :
            fgcolor := 7 + bright
          39 :
            fgcolor := 0
          40 :
            bgcolor := 0
          41 :
            bgcolor := 1
          42 :
            bgcolor := 2
          43 :
            bgcolor := 3
          44 :
            bgcolor := 4
          45 :
            bgcolor := 5
          46 :
            bgcolor := 6
          47 :
            bgcolor := 7
          49 :
            bgcolor := 0
        i := i + 1
      until not (i < pn)
      if (fgcolor == 0) and (bgcolor == 0)
        fbcolor := DEFCOLORS
      else
        fbcolor := (palette[bgcolor] << 8) | palette[fgcolor]
      
PUB Start | c, hold, blen

  ' Main program

  ' Start serial driver
  serial.COMEngineStart(RX, TX, BAUD)

  ' Start vga text driver
  vgatext.start(VGAB, @screen, @colors, @cx0, @sync)

  ' Set default screen colors, attribute, clear screen
  fgcolor := bgcolor := reverse := bright := 0
  fbcolor := DEFCOLORS
  Cls

  ' Cursor 0 a fast-blinking underscore, cursor 1 off
  cm0 := %111
  cx0 := 0
  cy0 := 0
  cm1 := %000

  ' Long beep
  dira[BELL_PINA] := 1
  dira[BELL_PINB] := 1
  Beep(1)

  ' Signal that we are ready
  dira[RDY] := 1
  outa[RDY] := 1
  dira[LED] := 1
  outa[LED] := 1

  ' Main loop
  autowrap := 1
  nlmode := 1
  hold := False
  repeat

    ' XON/XOFF handling
    blen := serial.receivedNumber
    if hold
      if blen < 64
        hold := False
        serial.writeByte(XON)
    else
      if not (blen < 192)
        hold := True
        serial.writeByte(XOFF)           

    ' Get character
    c := serial.readByte

    ' Control or data ?
    case c
      NUL :
      ENQ :
        pn := 0
        Passcmd(c)
        serial.writeString(@scres)
        serial.writeString(@ident)
        serial.writeByte(c)
      BEL :
        if not hold
          hold := True
          serial.writeByte(XOFF)
        Beep(0)
      BS :
        if cx0 <> 0
          --cx0
          Putb(SP)
      CR :
        cx0 := 0
      ESC :
        c := ESCcmd
        if c < 0
          Putb(c)
          ++cx0
      NL :
        ++cy0
        if nlmode
          cx0 := 0
      other :
        Putb(c)
        ++cx0

    ' Wrap and/or scroll
    if autowrap and (not (cx0 < cols))
      cx0 := 0
      ++cy0
    if cy0 == rows
      --cy0
      Scroll

DAT

' Screen resolution, commit identifiers
scres   byte "640x480 ", 0
ident   byte "$Id: 0123456789$", 0

' ANSI color palette
palette byte

byte %%000_0, %%200_0, %%020_0, %%210_0, %%002_0, %%202_0, %%022_0, %%222_0
byte %%111_0, %%311_0, %%131_0, %%331_0, %%113_0, %%313_0, %%133_0, %%333_0
byte %%000_0, %%200_0, %%020_0, %%210_0, %%002_0, %%202_0, %%022_0, %%222_0
byte %%111_0, %%311_0, %%131_0, %%331_0, %%113_0, %%313_0, %%133_0, %%333_0

{{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  TERMS OF USE: MIT License
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}}
