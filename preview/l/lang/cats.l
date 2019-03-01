; n-ary cats macro
(define cats
  (macro args
     ;(forget 'cats)
    `(n-ary cats _cats '"" ,@args)))
