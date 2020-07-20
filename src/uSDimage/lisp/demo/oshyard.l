; Evaluate infix arithmetic expressions, with precedence
(include '<makerlisp.l>)
(include '<stdio.l>)

(define oshyard
  (lambda ()

    ; Line buffer
    (define line)

    ; Character classes
    (define digitp (macro (c) `(not (zerop (isdigit (sym2int ,c))))))
    (define spacep (macro (c) `(not (zerop (isspace (sym2int ,c))))))

    ; Empty or non-empty string
    (define nils '"")
    (define slen (macro (s) `(not (eq ,s nils))))

    ; Delete last character from line
    (define delete
      (lambda ()
        (print '\b)
        (print '" ")
        (print '\b)
        (define l nils)
        (while (slen line)
          (if (not (slen (cdr line)))
              (break))
          (setq l (cats l (car line)))
          (setq line (cdr line)))

        (setq line l)))

    ; Get a line
    (define getln
      (lambda ()

        (define BS 0x08)
        (define DEL 0x7f)
        (define EOT 0x04)
        (define CR (sym2int '\r))
        (define NL (sym2int '\n))
        (define lastc)
        (define c)

        (setq line nils)
        (while 't
          (setq lastc (getchar))
          (if (= lastc CR)
              (continue))
          (if (or (= lastc NL) (= lastc EOT))
              (break))
          (if (or (= lastc BS) (= lastc DEL))
              (progn
                (delete)
                (continue)))

          (setq c (int2sym lastc))
          (print c)
          (setq line (cats line c)))

        (print '\n)))

    ; Get next token
    (define getoken
      (lambda op

        ; Skip leading white space
        (define t nils)
        (while (and (slen line) (spacep (car line)))
          (setq line (cdr line)))

        ; Until white space, or alpha/num change, or paren, copy into token
        (define c)
        (define d)
        (define o)
        (while (and (slen line) (not (spacep (setq c (car line)))))

          ; Set/check alpha/num type
          (setq d (digitp c))
          (if (not (slen t))
            (if (not op) (setq d (or d (eq c '+) (eq c '-))))
            (if (not (eq d o)) (break)))

          ; If character is '(' or ')' ...
          (if (or (eq c '"(") (eq c '")"))
            (progn

              ; Terminate current token
              (if (slen t)
                (break))

              ; If first, it is the token
              (setq t c)
              (setq line (cdr line))
              (break)))

          ; Copy character
          (setq t (cats t c))
          (setq line (cdr line))

          (setq o d))

        t))

    ; Get number from string
    (define getnum
      (lambda (s)

        ; If token is "(", evaluate sub-expression
        (if (eq s '"(")
          (xeval)

        ; Otherwise, scan the number
        (progn
          (define c (car s))
          (if (not (or (digitp c) (eq c '+) (eq c '-)))
            (error (cats '"\n? getnum: Bad number: '" s '"'\n")))
          (atoi s)))))

    ; Operators
    (define plus (lambda (a b) (+ a b)))
    (define minus (lambda (a b) (- a b)))
    (define times (lambda (a b) (* a b)))
    (define divide (lambda (a b) (/ a b)))
    (define power (lambda (a b) (real2int (pow (+ a 0.0) (+ b 0.0)))))

    (define MINPREC 0)
    (define ADDPREC 1)
    (define MULPREC 2)
    (define POWPREC 3)

    (define getassoc
      (lambda (p)
        (cond
         ((= p MINPREC) 'Left)
         ((= p ADDPREC) 'Left)
         ((= p MULPREC) 'Left)
         ((= p POWPREC) 'Right)
         ('t 'Left))))

    (define getfun
      (lambda (s)
        (case s
          ("" (list '() MINPREC))
          ("+" (list plus ADDPREC))
          ("-" (list minus ADDPREC))
          ("*" (list times MULPREC))
          ("/" (list divide MULPREC))
          ("^" (list power POWPREC))
          (t (error (cats '"\n? getfun: Unknown operator: '" s '"'\n"))))))

    ; Evaluate expression on line
    (define xeval
      (lambda ()

        ; Clear operations of equal (left assoc) or higher precedence
        (define clear
          (lambda (newf)
            (define newp)
            (define oldf)
            (define oldp)
            (define x)
            (define y)

            (while ((operators 'get))
              (setq oldf ((operators 'pop)))
              (setq oldp (cadr oldf))
              (setq newp (cadr newf))
              (if
                (or (< oldp newp) (and (= newp oldp) (eq (getassoc newp) 'Right)))
                  (progn
                    ((operators 'push) oldf)
                    (break)))
              (setq y ((operands 'pop)))
              (setq x ((operands 'pop)))
              ((operands 'push) ((car oldf) x y)))))

        ; Operand and operator stacks
        (define operands (stack))
        (define operators (stack))

        ; Get a number
        ((operands 'push) (getnum (getoken)))

        ; Find additional operators and operands
        (define f)
        (define k)
        (while 't

          ; End of expression ?
          (setq k (getoken 't))
          (if (or (not (slen k)) (eq k '")"))
            (break))

          ; Get the operator
          (setq f (getfun k))
          (clear f)
          ((operators 'push) f)

          ; Get the next number
          ((operands 'push) (getnum (getoken))))

        ; Clear remaining stacked up operations
        (clear (getfun '""))

        ; Return result
        ((operands 'pop))))

    ; Until blank line, get lines and evaluate expressions
    (print '">> ")
    (getln)
    (if (not (slen line))
      line
      (progn
        (print (xeval))
        (print '\n)
        (oshyard)))))
