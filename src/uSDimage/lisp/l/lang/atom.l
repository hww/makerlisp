; An atom is not a pair
(define atom
  (macro (x)
    (forget 'atom)
   `(not (pairp ,x))))
