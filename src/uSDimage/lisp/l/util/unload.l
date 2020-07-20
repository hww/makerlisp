; Forget a loaded function or macro
(define unload
  (lambda (x)

    (if (not (symbolp x))
      (error '"? \"unload\" expects one symbol as argument"))

    (define "*allow-forget*" 't)
    (forget x)))
