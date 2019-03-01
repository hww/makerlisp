; Arithmetic aliases
(if (boundp '_ARITHMETIC_L_) (eof) (define _ARITHMETIC_L_))

(include '<backquote.l>)

; Equality of n arguments
(define = (macro args `(equal ,@args)))

; Ascending order of n arguments
(define < (macro args `(less ,@args)))

; Descending order of n arguments
(define > (macro args `(greater ,@args)))

; Monotonic ascending order of n arguments
(define <= (macro args `(lessorequal ,@args)))

; Monotonic descending order of n arguments
(define >= (macro args `(greaterorequal ,@args)))

; Sum of n arguments
(define + (macro args `(add ,@args)))

; Difference of n arguments
(define - (macro args `(sub ,@args)))

; Product of n arguments
(define * (macro args `(mul ,@args)))

; Quotient of n arguments
(define / (macro args `(div ,@args)))

; Bitwise integer and of n arguments
(define & (macro args `(iand ,@args)))

; Bitwise integer or of n arguments
(define "|" (macro args `(ior ,@args)))

; Bitwise integer xor of n arguments
(define ^ (macro args `(ixor ,@args)))
