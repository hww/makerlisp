(define sqrt
  (macro args
    (forget 'sqrt)
   `(ffcall 'sqrt '(double (double)) (list ,@args))))
