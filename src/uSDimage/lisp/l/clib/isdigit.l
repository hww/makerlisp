(define isdigit
  (macro args
    (forget 'isdigit)
   `(ffcall 'isdigit '(int (int)) (list ,@args))))
