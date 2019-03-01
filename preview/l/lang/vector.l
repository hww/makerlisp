; Create a vector from arguments
(define vector
  (macro x
    (forget 'vector)
   `(list2vector (list ,@x))))
