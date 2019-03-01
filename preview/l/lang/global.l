; 'global' binds and assigns in the top environment
; (won't work if a local variable shadows the global variable)
(define global
  (macro (var . val)
    (forget 'global)

    (if val
     `(progn (eval '(define ,var) *topk*) (setq ,var ,@val))
     `(eval '(define ,var) *topk*))))
