(define rewind
  (macro args
    (forget 'rewind)
   `(ffcall 'rewind '(int (void*)) (list ,@args))))
