; Print expression followed by "\n"
(define nlprint
  (macro (x)
    (forget 'nlprint)
   `(progn (define d (defer 't)) (print ,x) (print '\n) (defer d) '"")))
