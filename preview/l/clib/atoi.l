(define atoi
  (macro args
    (forget 'atoi)
   `(ffcall 'atoi '(int (char*)) (list ,@args))))
