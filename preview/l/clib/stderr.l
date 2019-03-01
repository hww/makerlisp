(define stderr
  (macro args
    (forget 'stderr)
   `(ffcall 'stderr '(void* ()) (list ,@args))))
