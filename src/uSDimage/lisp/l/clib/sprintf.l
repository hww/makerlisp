(define sprintf
  (macro args
    (forget 'sprintf)
   `(ffcall 'sprintf '(char* (char* ?)) (list ,@args))))
