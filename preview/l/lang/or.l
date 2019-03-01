; Logical, short-circuiting or of n arguments
(define or
  (macro args
    (forget 'or)
   `(n-ary or _lor '() ,@args)))
