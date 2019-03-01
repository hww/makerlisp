(define hello
  (macro ()
    (forget 'hello)
    `'"Hello from MakerLisp !\n"))
