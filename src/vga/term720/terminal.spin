''
'' VGA terminal, Luther Johnson
''
'' Uses the "waitvid.80x30.c0df.driver.2048" VGA driver (thanks ! Marko Lukat),
'' the "Full Duplex COM Engine" (Kwabena W. Agyeman), and "Basic I2C Routines"
'' (Michael Green).
''
'' Inspired and informed by the Hobbytronics "Serial VGA", Marco Maccaferri's
'' "RC2014 VGA Terminal", and the Parallax "VGA High-Res Text Driver" (Chip
'' Gracey).

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000

  ' Screen geometry, from driver
  cols     = driver#res_x / 9
  rows     = driver#res_y / font#height
  bcnt     = cols * rows

  ' Screen buffer is laid out with bottom row at low memory
  rows_raw = (driver#res_y + font#height - 1) / font#height
  bcnt_raw = cols * rows_raw

  ' VGA pins
  vgrp     = 2                                          ' video pin group
  vpin     = %%333_0                                    ' video pin mask

  video    = (vgrp << 9 | vpin) << 21

  ' Cursor characteristics
  CURSOR_ON    = driver#CURSOR_ON
  CURSOR_OFF   = driver#CURSOR_OFF
  CURSOR_ULINE = driver#CURSOR_ULINE
  CURSOR_BLOCK = driver#CURSOR_BLOCK
  CURSOR_BLINK = driver#CURSOR_FLASH
  CURSOR_SOLID = driver#CURSOR_SOLID

  CURSOR_MASK  = driver#CURSOR_MASK

  #0, CM, CX, CY

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

OBJ
  driver: "waitvid.80x30.c0df.driver.2048"
    font: "generic8x16-4font"
  serial: "Full-Duplex_COMEngine"
     i2c: "BasicI2C"
  
VAR
  long  scrn[bcnt_raw / 2]      ' screen buffer
  long  link[driver#res_m]      ' mailbox

  long  cursor                  ' text cursor
  byte  fgcolor, bgcolor, reverse, bright
  byte  pn, p[8]
  byte  autowrap, nlmode
  long  romaddr                 ' Eeprom read/write address 

PRI Beep(lbeep) | blen

  ' Beep the speaker
  frqa := trunc(53.6870912 * float(BELL_FREQ))
  ctra := (%00100 << 26) | (BELL_PINB << 9) | BELL_PINA
  waitcnt(((SCLK_FREQ / 1000) * (1 + lbeep) * BELL_MS) + cnt)
  ctra := 0

PRI Rcolor

  ' Get fg, bg, reverse palette index
  result := (fgcolor << 4) | (bgcolor << 1) | reverse

PRI Cls | attr

  ' Clear the screen
  attr.byte[0] := Rcolor
  attr.byte[1] := SP
  wordfill(@scrn{0}, attr, bcnt_raw)
  
PRI CursorMode(setup)

  cursor.byte[CM] := (cursor.byte[CM] & constant(!CURSOR_MASK)) | setup

PRI Putb(c) | attr, x, y

  ' Stuff a character/attribute into the screen buffer at the cursor
  attr.byte[0] := Rcolor
  attr.byte[1] := c
  x := cursor.byte[CX]
  y := cursor.byte[CY]
  scrn.word[bcnt_raw - y*cols - ++x] := attr

PRI Scroll | attr

  ' Roll the screen up one line
  longmove(@scrn.word[cols], @scrn.word[0], ((rows - 1)*cols)/2)

  ' Blank the bottom line
  attr.byte[0] := Rcolor
  attr.byte[1] := SP
  wordfill(@scrn.word[0], attr, cols)

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
      cursor.byte[CY] := cursor.byte[CY] - i
      if not (cursor.byte[CY] < rows)
        cursor.byte[CY] := 0 
    "B" :
      ' Cursor down
      i := 1
      if (pn == 1)
        i := p[0]
      cursor.byte[CY] := cursor.byte[CY] + i
      if not (cursor.byte[CY] < rows)
        cursor.byte[CY] := rows - 1 
    "C" :
      ' Cursor forward
      i := 1
      if (pn == 1)
        i := p[0]
      cursor.byte[CX] := cursor.byte[CX] + i
      if not (cursor.byte[CX] < cols)
        cursor.byte[CX] := cols - 1 
    "D" :
      ' Cursor backward
      i := 1
      if (pn == 1)
        i := p[0]
      cursor.byte[CX] := cursor.byte[CX] - i
      if not (cursor.byte[CX] < cols)
        cursor.byte[CX] := 0 
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
        cursor.byte[CX] := p[1] - 1
        cursor.byte[CY] := p[0] - 1
      else
        cursor.byte[CX] := 0
        cursor.byte[CY] := 0
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
      ' Set fg/bg colors
      fg := serial.readByte
      bg := serial.readByte
      if (fg == $FF) and (bg == $FF)
        fgcolor := bgcolor := 0
      else
        fgcolor := fg & $0F
        bgcolor := bg & $07
    "Y" :
      ' Load palette entry
      fg := serial.readByte & $0F
      bg := serial.readByte & $07
      c := (serial.readByte << 10) | (serial.readByte << 2)
      palette[(fg*8 + bg)*2 + 0] := c
      palette[(fg*8 + bg)*2 + 1] := ((c & $0F) << 8) | ((c & $F0) >> 8)
    "Z" :
      ' Set reverse video flag
      reverse := serial.readByte & $01
    "?" :
      c := GetParms

      ' Turn cursor on or off
      if (pn == 1) and (p[0] == 25)
        if c == "h"
          CursorMode(constant(CURSOR_ON | CURSOR_ULINE | CURSOR_BLINK))
        if c == "l"
          CursorMode(constant(CURSOR_OFF))

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

PUB Start | c, hold, blen

  ' Main program

  ' Start serial driver
  serial.COMEngineStart(RX, TX, BAUD)

  ' Start I2C driver
  romaddr := 0
  i2c.Initialize(I2CSCL)

  ' Start vga text driver
  link{0} := video | @scrn{0}
  link[1] := @palette << 16 | font.addr
  link[2] := @cursor * $00010001
  driver.init(-1, @link{0})

  ' Clear screen, turn on cursor
  fgcolor := bgcolor := reverse := bright := 0
  Cls
  cursor.byte[CX] := cursor.byte[CY] := 0
  CursorMode(constant(CURSOR_ON | CURSOR_ULINE | CURSOR_BLINK))

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
        if cursor.byte[CX] <> 0
          --cursor.byte[CX]
          Putb(SP)
      CR :
        cursor.byte[CX] := 0
      ESC :
        c := ESCcmd
        if c < 0
          Putb(c)
          ++cursor.byte[CX]
      NL :
        ++cursor.byte[CY]
        if nlmode
          cursor.byte[CX] := 0
      other :
        Putb(c)
        ++cursor.byte[CX]

    ' Wrap and/or scroll
    if autowrap and (not (cursor.byte[CX] < cols))
      cursor.byte[CX] := 0
      ++cursor.byte[CY]
    if cursor.byte[CY] == rows
      --cursor.byte[CY]
      Scroll

DAT

' Screen resolution, commit identifiers
scres   byte "720x480 ", 0
ident   byte "$Id: 0123456789$", 0

' Palette of 16x16 fg, bg (ansi) color combinations, even entries are normal, odd entries are reversed 
palette word

        word    %%321_0_000_0, %%000_0_321_0  ' Default color instead of black on black

        word                                  %%000_0_200_0, %%200_0_000_0, %%000_0_020_0, %%020_0_000_0, %%000_0_210_0, %%210_0_000_0
        word    %%000_0_002_0, %%002_0_000_0, %%000_0_202_0, %%202_0_000_0, %%000_0_022_0, %%022_0_000_0, %%000_0_222_0, %%222_0_000_0
        word    %%200_0_000_0, %%000_0_200_0, %%200_0_200_0, %%200_0_200_0, %%200_0_020_0, %%020_0_200_0, %%200_0_210_0, %%210_0_200_0
        word    %%200_0_002_0, %%002_0_200_0, %%200_0_202_0, %%202_0_200_0, %%200_0_022_0, %%022_0_200_0, %%200_0_222_0, %%222_0_200_0
        word    %%020_0_000_0, %%000_0_020_0, %%020_0_200_0, %%200_0_020_0, %%020_0_020_0, %%020_0_020_0, %%020_0_210_0, %%210_0_020_0
        word    %%020_0_002_0, %%002_0_020_0, %%020_0_202_0, %%202_0_020_0, %%020_0_022_0, %%022_0_020_0, %%020_0_222_0, %%222_0_020_0
        word    %%210_0_000_0, %%000_0_210_0, %%210_0_200_0, %%200_0_210_0, %%210_0_020_0, %%020_0_210_0, %%210_0_210_0, %%210_0_210_0
        word    %%210_0_002_0, %%002_0_210_0, %%210_0_202_0, %%202_0_210_0, %%210_0_022_0, %%022_0_210_0, %%210_0_222_0, %%222_0_210_0
        word    %%002_0_000_0, %%000_0_002_0, %%002_0_200_0, %%200_0_002_0, %%002_0_020_0, %%020_0_002_0, %%002_0_210_0, %%210_0_002_0
        word    %%002_0_002_0, %%002_0_002_0, %%002_0_202_0, %%202_0_002_0, %%002_0_022_0, %%022_0_002_0, %%002_0_222_0, %%222_0_002_0
        word    %%202_0_000_0, %%000_0_202_0, %%202_0_200_0, %%200_0_202_0, %%202_0_020_0, %%020_0_202_0, %%202_0_210_0, %%210_0_202_0
        word    %%202_0_002_0, %%002_0_202_0, %%202_0_202_0, %%202_0_202_0, %%202_0_022_0, %%022_0_202_0, %%202_0_222_0, %%222_0_202_0
        word    %%022_0_000_0, %%000_0_022_0, %%022_0_200_0, %%200_0_022_0, %%022_0_020_0, %%020_0_022_0, %%022_0_210_0, %%210_0_022_0
        word    %%022_0_002_0, %%002_0_022_0, %%022_0_202_0, %%202_0_022_0, %%022_0_022_0, %%022_0_022_0, %%022_0_222_0, %%222_0_022_0
        word    %%222_0_000_0, %%000_0_222_0, %%222_0_200_0, %%200_0_222_0, %%222_0_020_0, %%020_0_222_0, %%222_0_210_0, %%210_0_222_0
        word    %%222_0_002_0, %%002_0_222_0, %%222_0_202_0, %%202_0_222_0, %%222_0_022_0, %%022_0_222_0, %%222_0_222_0, %%222_0_222_0
        word    %%111_0_000_0, %%000_0_111_0, %%111_0_200_0, %%200_0_111_0, %%111_0_020_0, %%020_0_111_0, %%111_0_210_0, %%210_0_111_0
        word    %%111_0_002_0, %%002_0_111_0, %%111_0_202_0, %%202_0_111_0, %%111_0_022_0, %%022_0_111_0, %%111_0_222_0, %%222_0_111_0
        word    %%311_0_000_0, %%000_0_311_0, %%311_0_200_0, %%200_0_311_0, %%311_0_020_0, %%020_0_311_0, %%311_0_210_0, %%210_0_311_0
        word    %%311_0_002_0, %%002_0_311_0, %%311_0_202_0, %%202_0_311_0, %%311_0_022_0, %%022_0_311_0, %%311_0_222_0, %%222_0_311_0
        word    %%131_0_000_0, %%000_0_131_0, %%131_0_200_0, %%200_0_131_0, %%131_0_020_0, %%020_0_131_0, %%131_0_210_0, %%210_0_131_0
        word    %%131_0_002_0, %%002_0_131_0, %%131_0_202_0, %%202_0_131_0, %%131_0_022_0, %%022_0_131_0, %%131_0_222_0, %%222_0_131_0
        word    %%331_0_000_0, %%000_0_331_0, %%331_0_200_0, %%200_0_331_0, %%331_0_020_0, %%020_0_331_0, %%331_0_210_0, %%210_0_331_0
        word    %%331_0_002_0, %%002_0_331_0, %%331_0_202_0, %%202_0_331_0, %%331_0_022_0, %%022_0_331_0, %%331_0_222_0, %%222_0_331_0
        word    %%113_0_000_0, %%000_0_113_0, %%113_0_200_0, %%200_0_113_0, %%113_0_020_0, %%020_0_113_0, %%113_0_210_0, %%210_0_113_0
        word    %%113_0_002_0, %%002_0_113_0, %%113_0_202_0, %%202_0_113_0, %%113_0_022_0, %%022_0_113_0, %%113_0_222_0, %%222_0_113_0
        word    %%313_0_000_0, %%000_0_313_0, %%313_0_200_0, %%200_0_313_0, %%313_0_020_0, %%020_0_313_0, %%313_0_210_0, %%210_0_313_0
        word    %%313_0_002_0, %%002_0_313_0, %%313_0_202_0, %%202_0_313_0, %%313_0_022_0, %%022_0_313_0, %%313_0_222_0, %%222_0_313_0
        word    %%133_0_000_0, %%000_0_133_0, %%133_0_200_0, %%200_0_133_0, %%133_0_020_0, %%020_0_133_0, %%133_0_210_0, %%210_0_133_0
        word    %%133_0_002_0, %%002_0_133_0, %%133_0_202_0, %%202_0_133_0, %%133_0_022_0, %%022_0_133_0, %%133_0_222_0, %%222_0_133_0
        word    %%333_0_000_0, %%000_0_333_0, %%333_0_200_0, %%200_0_333_0, %%333_0_020_0, %%020_0_333_0, %%333_0_210_0, %%210_0_333_0
        word    %%333_0_002_0, %%002_0_333_0, %%333_0_202_0, %%202_0_333_0, %%333_0_022_0, %%022_0_333_0, %%333_0_222_0, %%222_0_333_0

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
