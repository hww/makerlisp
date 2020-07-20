''***************************************
''*  VGA High-Res Text Driver v1.0      *
''*  Author: Chip Gracey                *
''*  Copyright (c) 2006 Parallax, Inc.  *
''*  See end of file for terms of use.  *
''***************************************

'' Modified by Luther Johnson
'' Use 8x16 font in x=640 by y=480, n=80 columns, m=30 rows,
'' or x=1024 by y=768, n=128 columns, m=48 rows.
''
'' Code page 437 font from generic-8x16-4font.spin by W.L.Schneider (zx81vga)
''
'' This object generates an x by y VGA signal which contains n columns by m
'' rows of 8x16 characters. Each row can have a unique foreground/background
'' color combination and each character can be inverted. There are also two
'' cursors which can be independently controlled (ie. mouse and keyboard). A
'' sync indicator signals each time the screen is refreshed (you may ignore).
''
'' You must provide buffers for the screen, colors, cursors, and sync. Once
'' started, all interfacing is done via memory. To this object, all buffers are
'' read-only, with the exception of the sync indicator which gets written with
'' -1. You may freely write all buffers to affect screen appearance. Have fun!
''

CON

{
' 1024 x 768 @ 57Hz settings: 128 x 48 characters

  hp = 1024     'horizontal pixels
  vp = 768      'vertical pixels
  hf = 16       'horizontal front porch pixels
  hs = 96       'horizontal sync pixels
  hb = 176      'horizontal back porch pixels
  vf = 1        'vertical front porch lines
  vs = 3        'vertical sync lines
  vb = 28       'vertical back porch lines
  hn = 1        'horizontal normal sync state (0|1)
  vn = 1        'vertical normal sync state (0|1)
  pr = 60       'pixel rate in MHz at 80MHz system clock (5MHz granularity)
'}
'{
' 640 x 480 @ 57Hz settings: 80 x 30 characters

  hp = 640      'horizontal pixels
  vp = 480      'vertical pixels
  hf = 24       'horizontal front porch pixels
  hs = 40       'horizontal sync pixels
  hb = 140      'horizontal back porch pixels
  vf = 9        'vertical front porch lines
  vs = 3        'vertical sync lines
  vb = 28       'vertical back porch lines
  hn = 1        'horizontal normal sync state (0|1)
  vn = 1        'vertical normal sync state (0|1)
  pr = 25       'pixel rate in MHz at 80MHz system clock (5MHz granularity)
'}

' columns and rows

  cols = hp / 8
  rows = vp / 16
  

VAR long cog[2]

PUB start(BasePin, ScreenPtr, ColorPtr, CursorPtr, SyncPtr) : okay | i, j

'' Start VGA driver - starts two COGs
'' returns false if two COGs not available
''
''     BasePin = VGA starting pin (0, 8, 16, 24, etc.)
''
''   ScreenPtr = Pointer to (n x m x 2) bytes containing ASCII characters,
''               and an invert attribute for each of the screen characters.
''               Screen memory is arranged left-to-right, top-to-bottom.
''
''               screen buffer word example: %1_01000001 = inverse "A"
''
''    ColorPtr = Pointer to m words which define the foreground and background
''               colors for each row. The lower byte of each word contains the
''               foreground RGB data for that row, while the upper byte
''               contains the background RGB data. The RGB data in each byte is
''               arranged as %RRGGBB00 (4 levels each).
''
''               color word example: %%0020_3300 = gold on blue
''
''   CursorPtr = Pointer to 6 bytes which control the cursors:
''
''               bytes 0,1,2: X, Y, and MODE of cursor 0
''               bytes 3,4,5: X, Y, and MODE of cursor 1
''
''               X and Y are in terms of screen characters
''               (left-to-right, top-to-bottom)
''
''               MODE uses three bottom bits:
''
''                      %x00 = cursor off
''                      %x01 = cursor on
''                      %x10 = cursor on, blink slow
''                      %x11 = cursor on, blink fast
''                      %0xx = cursor is solid block
''                      %1xx = cursor is underscore
''
''               cursor example: 127, 63, %010 = blinking block in lower-right
''
''     SyncPtr = Pointer to long which gets written with -1 upon each screen
''               refresh. May be used to time writes/scrolls, so that chopiness
''               can be avoided. You must clear it each time if you want to see
''               it re-trigger.

  'if driver is already running, stop it
  stop

  'implant pin settings
  reg_vcfg := $200000FF + (BasePin & %111000) << 6
  i := $FF << (BasePin & %011000)
  j := BasePin & %100000 == 0
  reg_dira := i & j
  reg_dirb := i & !j

  'implant CNT value to sync COGs to
  sync_cnt := cnt + $10000

  'implant pointers
  longmove(@screen_base, @ScreenPtr, 3)
  font_base := @font

  'implant unique settings and launch first COG
  vf_lines.byte := vf
  vb_lines.byte := vb
  font_fourth := 1
  cog[1] := cognew(@d0, SyncPtr) + 1

  'allow time for first COG to launch
  waitcnt($2000 + cnt)

  'differentiate settings and launch second COG
  vf_lines.byte := vf+4
  vb_lines.byte := vb-4
  font_fourth := 0
  cog[0] := cognew(@d0, SyncPtr) + 1

  'if both COGs launched, return true
  if cog[0] and cog[1]
    return true
    
  'else, stop any launched COG and return false
  else
    stop


PUB stop | i

'' Stop VGA driver - frees two COGs

  repeat i from 0 to 1
    if cog[i]
      cogstop(cog[i]~ - 1)


CON

  #1, scanbuff[128], scancode[128*2-1+3], maincode      'enumerate COG RAM usage

  main_size = $1F0 - maincode                           'size of main program   

  hv_inactive = (hn << 1 + vn) * $0101                  'H,V inactive states

  
DAT

'*****************************************************
'* Assembly language VGA high-resolution text driver *
'*****************************************************

' This program runs concurrently in two different COGs.
'
' Each COG's program has different values implanted for front-porch lines and
' back-porch lines which surround the vertical sync pulse lines. This allows
' timed interleaving of their active display signals during the visible portion
' of the field scan. Also, they are differentiated so that one COG displays
' even four-line groups while the other COG displays odd four-line groups.
'
' These COGs are launched in the PUB 'start' and are programmed to synchronize
' their PLL-driven video circuits so that they can alternately prepare sets of
' four scan lines and then display them. The COG-to-COG switchover is seamless
' due to two things: exact synchronization of the two video circuits and the
' fact that all COGs' driven output states get OR'd together, allowing one COG
' to output lows during its preparatory state while the other COG effectively
' drives the pins to create the visible and sync portions of its scan lines.
' During non-visible scan lines, both COGs output together in unison.
'
' COG RAM usage:  $000      = d0 - used to inc destination fields for indirection
'                 $001-$080 = scanbuff - longs which hold 4 scan lines
'                 $081-$182 = scancode - stacked WAITVID/SHR for fast display
'                 $183-$1EF = maincode - main program loop which drives display

                        org                             'set origin to $000 for start of program

d0                      long    1 << 9                  'd0 always resides here at $000, executes as NOP


' Initialization code and data - after execution, space gets reused as scanbuff

                        'Move main program into maincode area

:move                   mov     $1EF,main_begin+main_size-1                 
                        sub     :move,d0s0              '(do reverse move to avoid overwrite)
                        djnz    main_ctr,#:move                                     
                                                                                        
                        'Build scanbuff display routine into scancode                      
                                                                                        
:waitvid                mov     scancode+0,i0           'org     scancode                                              
:shr                    mov     scancode+1,i1           'waitvid color,scanbuff+0                    
                        add     :waitvid,d1             'shr     scanbuff+0,#8                       
                        add     :shr,d1                 'waitvid color,scanbuff+1                    
                        add     i0,#1                   'shr     scanbuff+1,#8                       
                        add     i1,d0                   '...                                         
                        djnz    scan_ctr,#:waitvid      'waitvid color,scanbuff+cols-1
                            
                        mov     scancode+cols*2-1,i2    'mov     vscl,#hf                            
                        mov     scancode+cols*2+0,i3    'waitvid hvsync,#0                           
                        mov     scancode+cols*2+1,i4    'jmp     #scanret                            
                                                                                 
                        'Init I/O registers and sync COGs' video circuits
                                                                                              
                        mov     dira,reg_dira           'set pin directions                   
                        mov     dirb,reg_dirb                                                 
                        movi    frqa,#(pr / 5) << 2     'set pixel rate                                      
                        mov     vcfg,reg_vcfg           'set video configuration
                        mov     vscl,#1                 'set video to reload on every pixel
                        waitcnt sync_cnt,colormask      'wait for start value in cnt, add ~1ms
                        movi    ctra,#%00001_110        'COGs in sync! enable PLLs now - NCOs locked!
                        waitcnt sync_cnt,#0             'wait ~1ms for PLLs to stabilize - PLLs locked!
                        mov     vscl,#100               'ensure initial WAITVIDs lock cleanly

                        'Jump to main loop
                        
                        jmp     #vsync                  'jump to vsync - WAITVIDs will now be locked!

                        'Data

d0s0                    long    1 << 9 + 1         
d1                      long    1 << 10
main_ctr                long    main_size
scan_ctr                long    cols

i0                      waitvid x,scanbuff+0
i1                      shr     scanbuff+0,#8
i2                      mov     vscl,#hf
i3                      waitvid hvsync,#0
i4                      jmp     #scanret

reg_dira                long    0                       'set at runtime
reg_dirb                long    0                       'set at runtime
reg_vcfg                long    0                       'set at runtime
sync_cnt                long    0                       'set at runtime

                        'Directives

                        fit     scancode                'make sure initialization code and data fit
main_begin              org     maincode                'main code follows (gets moved into maincode)


' Main loop, display field - each COG alternately builds and displays four scan lines
                          
vsync                   mov     x,#vs                   'do vertical sync lines
                        call    #blank_vsync

vb_lines                mov     x,#vb                   'do vertical back porch lines (# set at runtime)
                        call    #blank_vsync

                        mov     screen_ptr,screen_base  'reset screen pointer to upper-left character
                        mov     color_ptr,color_base    'reset color pointer to first row
                        mov     row,#0                  'reset row counter for cursor insertion
                        mov     fours,#rows * 4 / 2     'set number of 4-line builds for whole screen
                        
                        'Build four scan lines into scanbuff

fourline                mov     font_ptr,font_fourth    'get address of appropriate font section
                        shl     font_ptr,#8+2
                        add     font_ptr,font_base
                        
                        movd    :pixa,#scanbuff-1       'reset scanbuff address (pre-decremented)
                        movd    :pixb,#scanbuff-1
                        
                        mov     y,#2                    'must build scanbuff in two sections because
                        mov     vscl,vscl_line2x        '..pixel counter is limited to twelve bits

:halfrow                waitvid underscore,#0           'output lows to let other COG drive VGA pins
                        mov     x,#cols/2               '..for 2 scan lines, ready for half a row
                        
:column                 rdword  z,screen_ptr            'get character, attribute from screen memory
                        cmpsub  z,#$100         wc      'set inverse flag
                        shl     z,#2                    'index to character's font entry
                        add     z,font_ptr              'add font section address to point to 8*4 pixels
                        add     :pixa,d0                'increment scanbuff destination addresses
                        add     :pixb,d0
                        add     screen_ptr,#2           'increment screen memory address
:pixa                   rdlong  scanbuff,z              'read pixel long (8*4) into scanbuff
:pixb   if_nc           xor     scanbuff,longmask       'invert pixels according to inverse flag
                        djnz    x,#:column              'another character in this half-row?

                        djnz    y,#:halfrow             'loop to do 2nd half-row, time for 2nd WAITVID

                        sub     screen_ptr,#cols*2      'back up to start of same row in screen memory

                        'Insert cursors into scanbuff

                        mov     z,#2                    'ready for two cursors

:cursor                 rdbyte  x,cursor_base           'x in range?
                        add     cursor_base,#1
                        cmp     x,#cols         wc
                        
                        rdbyte  y,cursor_base           'y match?
                        add     cursor_base,#1
                        cmp     y,row           wz

                        rdbyte  y,cursor_base           'get cursor mode
                        add     cursor_base,#1

        if_nc_or_nz     jmp     #:nocursor              'if cursor not in scanbuff, no cursor

                        add     x,#scanbuff             'cursor in scanbuff, set scanbuff address
                        movd    :xor,x

                        test    y,#%010         wc      'get mode bits into flags
                        test    y,#%001         wz
        if_nc_and_z     jmp     #:nocursor              'if cursor disabled, no cursor
        
        if_c_and_z      test    slowbit,cnt     wc      'if blink mode, get blink state
        if_c_and_nz     test    fastbit,cnt     wc

                        test    y,#%100         wz      'get box or underscore cursor piece
        if_z            mov     x,longmask          
        if_nz           mov     x,underscore
        if_nz           cmp     font_fourth,#3  wz      'if underscore, must be last font section

:xor    if_nc_and_z     xor     scanbuff,x              'conditionally xor cursor into scanbuff

:nocursor               djnz    z,#:cursor              'second cursor?

                        sub     cursor_base,#3*2        'restore cursor base

                        'Display four scan lines from scanbuff

                        rdword  x,color_ptr             'get color pattern for current row
                        and     x,colormask             'mask away hsync and vsync signal states
                        or      x,hv                    'insert inactive hsync and vsync states

                        mov     y,#4                    'ready for four scan lines

scanline                mov     vscl,vscl_chr           'set pixel rate for characters
                        jmp     #scancode               'jump to scanbuff display routine in scancode
scanret                 mov     vscl,#hs                'do horizontal sync pixels
                        waitvid hvsync,#1               '#1 makes hsync active
                        mov     vscl,#hb                'do horizontal back porch pixels
                        waitvid hvsync,#0               '#0 makes hsync inactive
                        shr     scanbuff+cols-1,#8      'shift last column's pixels right by 8
                        djnz    y,#scanline             'another scan line?

                        'Next group of four scan lines
                        
                        add     font_fourth,#2          'if font_fourth + 2 => 3, subtract 3 (new row)
                        cmpsub  font_fourth,#4   wc     'c=0 for same row, c=1 for new row
        if_c            add     screen_ptr,#cols*2      'if new row, advance screen pointer
        if_c            add     color_ptr,#2            'if new row, advance color pointer
        if_c            add     row,#1                  'if new row, increment row counter
                        djnz    fours,#fourline         'another 4-line build/display?

                        'Visible section done, do vertical sync front porch lines

                        wrlong  longmask,par            'write -1 to refresh indicator
                        
vf_lines                mov     x,#vf                   'do vertical front porch lines (# set at runtime)
                        call    #blank

                        jmp     #vsync                  'new field, loop to vsync

                        'Subroutine - do blank lines

blank_vsync             xor     hvsync,#$101            'flip vertical sync bits

blank                   mov     vscl,hx                 'do blank pixels
                        waitvid hvsync,#0
                        mov     vscl,#hf                'do horizontal front porch pixels
                        waitvid hvsync,#0
                        mov     vscl,#hs                'do horizontal sync pixels
                        waitvid hvsync,#1
                        mov     vscl,#hb                'do horizontal back porch pixels
                        waitvid hvsync,#0
                        djnz    x,#blank                'another line?
blank_ret
blank_vsync_ret         ret

                        'Data

screen_base             long    0                       'set at runtime (3 contiguous longs)
color_base              long    0                       'set at runtime    
cursor_base             long    0                       'set at runtime

font_base               long    0                       'set at runtime
font_fourth             long    0                       'set at runtime

hx                      long    hp                      'visible pixels per scan line
vscl_line2x             long    (hp + hf + hs + hb) * 2 'total number of pixels per 2 scan lines
vscl_chr                long    1 << 12 + 8             '1 clock per pixel and 8 pixels per set
colormask               long    $FCFC                   'mask to isolate R,G,B bits from H,V
longmask                long    $FFFFFFFF               'all bits set
slowbit                 long    1 << 25                 'cnt mask for slow cursor blink
fastbit                 long    1 << 24                 'cnt mask for fast cursor blink
underscore              long    $FEFE0000               'underscore cursor pattern
hv                      long    hv_inactive             '-H,-V states
hvsync                  long    hv_inactive ^ $200      '+/-H,-V states

                        'Uninitialized data

screen_ptr              res     1
color_ptr               res     1
font_ptr                res     1

x                       res     1
y                       res     1
z                       res     1

row                     res     1
fours                   res     1


' 8 x 16 font - characters 0..255
'
' Each long holds four scan lines of a single character. The longs are arranged into
' 8 groups of 128 for both standard (0..127) and "extended" ascii (128..255), or four
' groups of 256, which each contain a vertical quarter of all characters, ordered from
' top to bottom.

font  long

'First 4 Scanlines of Char 00h - 7Fh

long  $00000000,$817E0000,$FF7E0000,$00000000,$00000000,$18000000,$18000000,$00000000
long  $FFFFFFFF,$00000000,$FFFFFFFF,$70780000,$663C0000,$CCFC0000,$C6FE0000,$18000000
long  $07030100,$70604000,$3C180000,$66660000,$DBFE0000,$06633E00,$00000000,$3C180000
long  $3C180000,$18180000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$3C180000,$66666600,$36000000,$633E1818,$00000000,$361C0000,$0C0C0C00
long  $18300000,$180C0000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $361C0000,$1C180000,$633E0000,$633E0000,$38300000,$037F0000,$061C0000,$637F0000
long  $633E0000,$633E0000,$00000000,$00000000,$60000000,$00000000,$06000000,$633E0000
long  $3E000000,$1C080000,$663F0000,$663C0000,$361F0000,$667F0000,$667F0000,$663C0000
long  $63630000,$183C0000,$30780000,$66670000,$060F0000,$77630000,$67630000,$633E0000
long  $663F0000,$633E0000,$663F0000,$633E0000,$7E7E0000,$63630000,$63630000,$63630000
long  $63630000,$66660000,$637F0000,$0C3C0000,$01000000,$303C0000,$63361C08,$00000000
long  $30180C00,$00000000,$06070000,$00000000,$30380000,$00000000,$6C380000,$00000000
long  $06070000,$18180000,$60600000,$06070000,$181C0000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$0C080000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$18700000,$18180000,$180E0000,$003B6E00,$00000000

'First 4 Scanlines of Char 80h - FFh

long  $663C0000,$00330000,$0C183000,$361C0800,$00330000,$180C0600,$1C361C00,$00000000
long  $361C0800,$00630000,$180C0600,$00660000,$663C1800,$180C0600,$08006300,$081C361C
long  $7F001830,$00000000,$367C0000,$361C0800,$00630000,$180C0600,$331E0C00,$180C0600
long  $00630000,$3E006300,$63006300,$3E181800,$26361C00,$66660000,$33331F00,$18D87000
long  $060C1800,$0C183000,$060C1800,$060C1800,$3B6E0000,$63003B6E,$363C0000,$361C0000
long  $0C0C0000,$00000000,$00000000,$46070600,$46070600,$18180000,$00000000,$00000000
long  $22882288,$55AA55AA,$EEBBEEBB,$18181818,$18181818,$18181818,$6C6C6C6C,$00000000
long  $00000000,$6C6C6C6C,$6C6C6C6C,$00000000,$6C6C6C6C,$6C6C6C6C,$18181818,$00000000
long  $18181818,$18181818,$00000000,$18181818,$00000000,$18181818,$18181818,$6C6C6C6C
long  $6C6C6C6C,$00000000,$6C6C6C6C,$00000000,$6C6C6C6C,$00000000,$6C6C6C6C,$18181818
long  $6C6C6C6C,$00000000,$00000000,$6C6C6C6C,$18181818,$00000000,$00000000,$6C6C6C6C
long  $18181818,$18181818,$00000000,$FFFFFFFF,$00000000,$0F0F0F0F,$F0F0F0F0,$FFFFFFFF
long  $00000000,$331E0000,$637F0000,$00000000,$637F0000,$00000000,$00000000,$00000000
long  $187E0000,$361C0000,$361C0000,$0C780000,$00000000,$C0000000,$0C380000,$3E000000
long  $00000000,$00000000,$0C000000,$30000000,$D8700000,$18181818,$00000000,$00000000
long  $36361C00,$00000000,$00000000,$3030F000,$6C6C3600,$30663C00,$00000000,$00000000

'Second 4 Scanlines of Char 00h - 7Fh

long  $00000000,$BD8181A5,$C3FFFFDB,$7F7F7F36,$7F3E1C08,$E7E73C3C,$FFFF7E3C,$3C180000
long  $C3E7FFFF,$42663C00,$BD99C3FF,$331E4C58,$3C666666,$0C0C0CFC,$C6C6C6FE,$E73CDB18
long  $1F7F1F0F,$7C7F7C78,$1818187E,$66666666,$D8DEDBDB,$6363361C,$00000000,$1818187E
long  $1818187E,$18181818,$7F301800,$7F060C00,$03030000,$7F361400,$3E1C1C08,$3E3E7F7F
long  $00000000,$18183C3C,$00000024,$36367F36,$603E0343,$18306343,$3B6E1C36,$00000006
long  $0C0C0C0C,$30303030,$FF3C6600,$7E181800,$00000000,$7F000000,$00000000,$18306040
long  $6B6B6363,$1818181E,$0C183060,$603C6060,$7F33363C,$603F0303,$633F0303,$18306060
long  $633E6363,$607E6363,$00001818,$00001818,$060C1830,$00007E00,$6030180C,$18183063
long  $7B7B6363,$7F636336,$663E6666,$03030343,$66666666,$161E1646,$161E1646,$7B030343
long  $637F6363,$18181818,$30303030,$1E1E3666,$06060606,$636B7F7F,$737B7F6F,$63636363
long  $063E6666,$63636363,$363E6666,$301C0663,$1818185A,$63636363,$63636363,$6B6B6363
long  $1C1C3E36,$183C6666,$0C183061,$0C0C0C0C,$1C0E0703,$30303030,$00000000,$00000000
long  $00000000,$3E301E00,$66361E06,$03633E00,$33363C30,$7F633E00,$0C1E0C4C,$33336E00
long  $666E3606,$18181C00,$60607000,$1E366606,$18181818,$6B7F3700,$66663B00,$63633E00
long  $66663B00,$33336E00,$666E3B00,$06633E00,$0C0C3F0C,$33333300,$63636300,$6B636300
long  $1C366300,$63636300,$18337F00,$180E1818,$18181818,$18701818,$00000000,$63361C08

'Second 4 Scanlines of Char 80h - FFh

long  $03030343,$33333300,$7F633E00,$3E301E00,$3E301E00,$3E301E00,$3E301E00,$03633E00
long  $7F633E00,$7F633E00,$7F633E00,$18181C00,$18181C00,$18181C00,$6363361C,$7F63361C
long  $1E164666,$6C6C3700,$337F3333,$63633E00,$63633E00,$63633E00,$33333300,$33333300
long  $63636300,$63636363,$63636363,$03030363,$06060F06,$187E183C,$7B33231F,$187E1818
long  $3E301E00,$18181C00,$63633E00,$33333300,$66663B00,$7B7F6F67,$7E007C36,$3E001C36
long  $060C0C00,$037F0000,$607F0000,$0C183666,$0C183666,$18181800,$1B366C00,$6C361B00
long  $22882288,$55AA55AA,$EEBBEEBB,$18181818,$1F181818,$1F181F18,$6F6C6C6C,$7F000000
long  $1F181F00,$6F606F6C,$6C6C6C6C,$6F607F00,$7F606F6C,$7F6C6C6C,$1F181F18,$1F000000
long  $F8181818,$FF181818,$FF000000,$F8181818,$FF000000,$FF181818,$F818F818,$EC6C6C6C
long  $FC0CEC6C,$EC0CFC00,$FF00EF6C,$EF00FF00,$EC0CEC6C,$FF00FF00,$EF00EF6C,$FF00FF18
long  $FF6C6C6C,$FF00FF00,$FF000000,$FC6C6C6C,$F818F818,$F818F800,$FC000000,$FF6C6C6C
long  $FF18FF18,$1F181818,$F8000000,$FFFFFFFF,$FF000000,$0F0F0F0F,$F0F0F0F0,$00FFFFFF
long  $1B3B6E00,$331B3333,$03030363,$36367F00,$18180C06,$1B1B7E00,$66666600,$18183B6E
long  $6666663C,$637F6363,$36636363,$667C3018,$DBDB7E00,$DBDB7E60,$063E0606,$63636363
long  $7F00007F,$187E1818,$30603018,$0C060C18,$181818D8,$18181818,$7E001800,$003B6E00
long  $0000001C,$18000000,$18000000,$37303030,$006C6C6C,$007E4C18,$7E7E7E7E,$00000000

'Third 4 Scanlines of Char 00h - 7Fh

long  $00000000,$7E818199,$7EFFFFE7,$081C3E7F,$00081C3E,$3C1818E7,$3C18187E,$0000183C
long  $FFFFE7C3,$003C6642,$FFC399BD,$1E333333,$18187E18,$070F0E0C,$67E7E6C6,$1818DB3C
long  $0103070F,$40607078,$00183C7E,$66660066,$D8D8D8D8,$63301C36,$7F7F7F7F,$7E183C7E
long  $18181818,$183C7E18,$00001830,$00000C06,$00007F03,$00001436,$007F7F3E,$00081C1C
long  $00000000,$18180018,$00000000,$36367F36,$3E636160,$6163060C,$6E333333,$00000000
long  $30180C0C,$0C183030,$0000663C,$00001818,$18181800,$00000000,$18180000,$0103060C
long  $1C366363,$7E181818,$7F630306,$3E636060,$78303030,$3E636060,$3E636363,$0C0C0C0C
long  $3E636363,$1E306060,$00181800,$0C181800,$6030180C,$0000007E,$060C1830,$18180018
long  $3E033B7B,$63636363,$3F666666,$3C664303,$1F366666,$7F664606,$0F060606,$5C666363
long  $63636363,$3C181818,$1E333333,$67666636,$7F664606,$63636363,$63636363,$3E636363
long  $0F060606,$3E7B6B63,$67666666,$3E636360,$3C181818,$3E636363,$081C3663,$36777F6B
long  $6363363E,$3C181818,$7F634306,$3C0C0C0C,$40607038,$3C303030,$00000000,$00000000
long  $00000000,$6E333333,$3E666666,$3E630303,$6E333333,$3E630303,$1E0C0C0C,$3E333333
long  $67666666,$3C181818,$60606060,$6766361E,$3C181818,$636B6B6B,$66666666,$3E636363
long  $3E666666,$3E333333,$0F060606,$3E63301C,$386C0C0C,$6E333333,$1C366363,$367F6B6B
long  $63361C1C,$7E636363,$7F63060C,$70181818,$18181818,$0E181818,$00000000,$007F6363

'Third 4 Scanlines of Char 80h - FFh

long  $3C664303,$6E333333,$3E630303,$6E333333,$6E333333,$6E333333,$6E333333,$3E630303
long  $3E630303,$3E630303,$3E630303,$3C181818,$3C181818,$3C181818,$6363637F,$63636363
long  $7F664616,$761B1B7E,$73333333,$3E636363,$3E636363,$3E636363,$6E333333,$6E333333
long  $7E636363,$3E636363,$3E636363,$18183E63,$3F670606,$1818187E,$63333333,$0E1B1818
long  $6E333333,$3C181818,$3E636363,$6E333333,$66666666,$63636373,$00000000,$00000000
long  $3E636303,$00030303,$00606060,$30613B06,$FC597366,$183C3C3C,$00006C36,$00001B36
long  $22882288,$55AA55AA,$EEBBEEBB,$18181818,$18181818,$18181818,$6C6C6C6C,$6C6C6C6C
long  $18181818,$6C6C6C6C,$6C6C6C6C,$6C6C6C6C,$00000000,$00000000,$00000000,$18181818
long  $00000000,$00000000,$18181818,$18181818,$00000000,$18181818,$18181818,$6C6C6C6C
long  $00000000,$6C6C6C6C,$00000000,$6C6C6C6C,$6C6C6C6C,$00000000,$6C6C6C6C,$00000000
long  $00000000,$18181818,$6C6C6C6C,$00000000,$00000000,$18181818,$6C6C6C6C,$6C6C6C6C
long  $18181818,$00000000,$18181818,$FFFFFFFF,$FFFFFFFF,$0F0F0F0F,$F0F0F0F0,$00000000
long  $6E3B1B1B,$33636363,$03030303,$36363636,$7F63060C,$0E1B1B1B,$3E666666,$18181818
long  $7E183C66,$1C366363,$77363636,$3C666666,$00007EDB,$03067ECF,$380C0606,$63636363
long  $007F0000,$7E000018,$7E000C18,$7E003018,$18181818,$1B1B1B18,$00001800,$00003B6E
long  $00000000,$00000018,$00000000,$383C3636,$00000000,$00000000,$007E7E7E,$00000000

'Fourth 4 Scanlines of Char 00h - 7Fh

long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $FFFFFFFF,$00000000,$FFFFFFFF,$00000000,$00000000,$00000000,$00000003,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$0000003E,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00001818,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$0000000C,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00007030,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$0000FF00
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$001E3330
long  $00000000,$00000000,$003C6666,$00000000,$00000000,$00000000,$00000000,$00000000
long  $000F0606,$00783030,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$001F3060,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000

'Fourth 4 Scanlines of Char 80h - FFh

long  $00000E18,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000E18
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $001E3060,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00007C18,$00006060,$00000000,$00000000,$00000000
long  $22882288,$55AA55AA,$EEBBEEBB,$18181818,$18181818,$18181818,$6C6C6C6C,$6C6C6C6C
long  $18181818,$6C6C6C6C,$6C6C6C6C,$6C6C6C6C,$00000000,$00000000,$00000000,$18181818
long  $00000000,$00000000,$18181818,$18181818,$00000000,$18181818,$18181818,$6C6C6C6C
long  $00000000,$6C6C6C6C,$00000000,$6C6C6C6C,$6C6C6C6C,$00000000,$6C6C6C6C,$00000000
long  $00000000,$18181818,$6C6C6C6C,$00000000,$00000000,$18181818,$6C6C6C6C,$6C6C6C6C
long  $18181818,$00000000,$18181818,$FFFFFFFF,$FFFFFFFF,$0F0F0F0F,$F0F0F0F0,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00030606,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$18181818,$0000000E,$00000000,$00000000
long  $00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000,$00000000

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
