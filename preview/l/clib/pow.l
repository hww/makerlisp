(define pow
  (macro args
    (forget 'pow)
   `(ffcall 'pow '(double (double double)) (list ,@args))))
