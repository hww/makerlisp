(define stdin
  (macro args
    (forget 'stdin)
   `(ffcall 'stdin '(void* ()) (list ,@args))))
