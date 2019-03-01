(define getenv
  (macro args
    (forget 'getenv)
   `(ffcall 'getenv '(char* (char*)) (list ,@args))))
