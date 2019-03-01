# Abstract

The MakerLisp Machine is a portable, modular, computer system, designed to recapture the feel of classic computing, with modern hardware.

The machine centers on a 2” x 3.5” CPU "business card", based on a 50 MHz Zilog eZ80, which can be used stand-alone, or plugged in to a 2” x 8” main board, for expansion to a full computer system. A laser-cut wood enclosure holds a small keyboard, an LCD monitor, the circuit boards, and a prototyping area with a breadboard for electronics experimentation and development.

The system software is 'MakerLisp', an open source implementation of a 'bare metal' Lisp, for Maker/DIY projects. MakerLisp has the Scheme evaluation model, Common Lisp-style low-level macros and primitive functions, and C language arithmetic and standard library functions. The system is written in portable C, and just-in-time compiles a tiny core Lisp language to SECD VM instructions. All higher-level forms are provided via Lisp functions and macros.

In this talk, we'll discuss and demonstrate MakerLisp on the machine, reviewing the language feature and implementation choices that were made in order to support an expressive and extensible dialect of Lisp, with good performance, in a small system.

More information is available at:

  https://makerlisp.com/

and on Facebook at:

  https://www.facebook.com/makerlisp

The slides for this talk are available under Documentation on that page.  A source preview is available at:

  https://makerlisp.com/source-preview
  
## MakerLisp Source Preview

To build a MakerLisp demonstration on Linux:

Unzip preview1.zip, the contents will be deposited in ./preview. 

In ./preview, compile: cc -olisp -O3 -Wall -I. lisp.c platform.c -lm

Start: ./lisp

There are demonstration programs/functions in ./preview, for example, 'cat.l', 'hello.l', 'fact.l', and 'sieve.l'. At the MakerLisp (">") prompt, start a program by using the name of the file, minus the extension, as the function name, for example "(hello)", "(fact 4)", "(sieve 30)", or "(cat 'cat.l)".

'lisp.c.txt" is posted here as a reading reference, it is identical to lisp.c in ./preview.