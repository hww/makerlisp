; backquote macro implementation
(if (boundp '_BACKQUOTE_L_) (eof) (define _BACKQUOTE_L_))

; Two arg append needed globally by backquote, also used by n-ary append macro
(define _append
  (lambda (front back)
    (if (not front)
      back
      (progn
        (if (not (pairp front))
          (error '"? All but last argument of \"_append\" must be lists"))
        (cons (car front) (_append (cdr front) back))))))

; One level backquote

; Construct an expression that builds the expression - when executed,
; unquoted things will be evaluated in the environment of the caller
(define _backquote
  (lambda (x)

    (if (not (pairp x))
      (list 'quote x)
      
      (if (eq (car x) 'unquote)
        (car (cdr x))
        
        (if (if (pairp (car x)) (eq (car (car x)) 'unquote-splicing))
          (list '_append (car (cdr (car x))) (_backquote (cdr x)))

          (list 'cons (_backquote (car x)) (_backquote (cdr x))))))))

(define backquote
  (macro (x)
    (_backquote x)))
