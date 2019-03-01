(define isspace
  (macro args
    (forget 'isspace)
   `(ffcall 'isspace '(int (int)) (list ,@args))))
