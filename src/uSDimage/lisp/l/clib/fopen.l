(define fopen
  (macro args
    (forget 'fopen)
   `(ffcall 'fopen '(void* (char* char*)) (list ,@args))))
