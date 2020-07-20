(define floor
  (macro args
    (forget 'floor)
   `(ffcall 'floor '(double (double)) (list ,@args))))
