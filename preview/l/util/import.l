; Import a function, lambda, or macro
(define import
  (macro (f)
    (forget 'import)
    (define file (cats f '".l"))
   `(global ,f (macro args `(loadapply ,file ,f ,',@args)))))
