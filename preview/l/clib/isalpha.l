(define isalpha
  (macro args
    (forget 'isalpha)
   `(ffcall 'isalpha '(int (int)) (list ,@args))))
