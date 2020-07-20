; Standard I/O streams
(if (boundp '_STDIO_L_) (eof) (define _STDIO_L_))

(discard (define STDIN (stdin)))
(discard (define STDOUT (stdout)))
(discard (define STDERR (stderr)))
(define EOF -1)
