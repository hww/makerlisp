(define ceil
  (macro args
    (forget 'ceil)
   `(ffcall 'ceil '(double (double)) (list ,@args))))
