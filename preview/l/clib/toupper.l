(define toupper
  (macro args
    (forget 'toupper)
   `(ffcall 'toupper '(int (int)) (list ,@args))))
