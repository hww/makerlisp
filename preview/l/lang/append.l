; n-ary append macro
(define append
  (macro args
     (forget 'append)
    `(n-ary append _append '() ,@args)))
