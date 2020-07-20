#!lisp.sh

#! Test basic functionality
(define fact 
  (lambda (x) 
    (if (zerop x) 1 (* x (fact (- x 1))))))

(define fib
  (lambda (x)
    (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))

(define faddn
  (lambda (n)
    (lambda (x) (+ x n))))

(define testcc
  (lambda (x)
    (+ 1 (call/cc (lambda (k) (setq r k) x)))))

(define r)
(testcc 1)
testcc
r
(r 22)
x
(define x)
x
(define y 0)
(setq x 1)
(setq x (+ x 1))
(setq y x)
(fact 9)
(fib 10)
(define add1 (faddn 1))
(define sub1 (faddn -1))
(add1 9)
(sub1 9)
'This\ is\ a\ symbol\ with\ spaces
'"This is \"another\" way to do it"
'[1 2 3]
[1 2 3]
[a b c]
(define v [1 2 (a b c) 3])
v
[]
(vref v 2)

; These were all added to fill out coverage
(())
(car ())
(cdr ())
(listp (list 'a 'b 'c))
(listp 'a)
(listp '())
(listp '(1 . 2 ))
(+ 'a 'b)
(- 'a 'b)
(zerop 'a)
(include 0)
(call/cc 1 2)
???
(define 1 2 3)
(define _add _add)
(if 1)
(lambda)
(macro)
(quote 1 2)
(setq 1 2 3)
(setq ??? 1)
(+ 1 2 3)
(_add 1 2 3)
(define test (lambda (a b c d) a))
(test 1 2 3 4)
test
123+456
123.a
1111111111111111111111111111111111111111111111111111111111111111111111111111111111111
++1
--1
1..2
0x123456
0xfFfF
0x100000
'g;1
'"g;1"
'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
( . b)
(a . )
.a

;
#!
; Comments
"
(()
(()))
'"\t\r\n"
` 
x(
)
()x
'(a .b)
(define .x 1)
. 
'(a b c . d)
'(a b c . d e)
(define test (lambda () (progn (progn 1))))
(test)
test
(< 'a 'b)
(+ 1 (progn 1 2 3))
(/ 1 0)
(/ 1 0.0)
(- 1 0.0)
(- 0.0 1)
(< 1 0.0)
(< 0.0 1)
(* 1 1.0)
(zerop 0)
(zerop 0.0)
(car 'a)
(cdr 1)
(+ 1 1.0)
(/ 4 2)
(/ 4.0 2)
(/ 'a 2)
(* 2.0 2)
(* 'a 'b)
(vref 'a 1)
(vref [] 0)
(vector2list 'a)
(list2vector 'a)
(= 1 1)
(= 1 1.0)
(= 1.0 1)
(= 1.0 1.0)
(= 'a 'a)
(define test (lambda (x) (progn (define y) (setq y 1) (+ x y))))
(test 1)
test
(define test (lambda () (lambda ())))
(test)
test
(if () ())
(define test (lambda () ???))
(test)
test
(define oldcar car)
(define test (lambda () (setq car cdr)))
(test)
(setq car oldcar)
(define test (lambda () (setq ??? 1)))
(test)
(| 1 2)
(| 1.0 2.0)
(& -1 -256)
(% -5 2)
(% 5.0 2)
(% 1 0)
(append '(1 2) '(3 4) '(5 6) '(7 8) (list 9 10))
(+)
(+ -1)
(+ 1 2 3 4 5 6 7 8 9 10)
(*)
(* 99)
(* 1 2 3 4)
(- 1)
(- 55 1 2 3 4 5 6 7 8 9)
(/ 2)
(/ 1.0 2 2 2)
(define one 1.0)
(length '(1 2 3))
(length 'a)
(boundp '?)
(assignedp '?)
(define ?)
(boundp '?)
(assignedp '?)
(setq ? '?)
(assignedp '?)
1.
1.2a
1.2.3
(realp 1.2)
(symbolp 'a)
(vectorp [1 2 3])
(define test (lambda () (define x) (setq x 1)))
(test)
test
(cats 'a 'b)
(cats 1 2)
(define s 'a)
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s s))
(setq s (cats s 'a))
(cats 'abc)
(cdr 'abc)
(define ifmacro (macro () '(if 1 1 1)))
(define test (lambda () (atom (ifmacro))))
(test)
test
((macro (n) `(+ ,n ,n)) 2)
(define x '(1 2))
(define y '(3 4))
((macro (a b) `(append ,a ,b)) x y)
((macro (a b) `(append ,a ,b)) 'x 'y)
(define f (lambda() _add))
((f))
((f) 1)
((f) 1 2)
((f) 1 2 3)
(nth 1 '(1 2 3))
nth
(nth 1 '(1 2 3))
(setetop '*mark* '())
(define _add)
_add
(define t t)
(forget '????)
(setetop '???? '())
(eval 1 2)
(*topk* 0)
(*topk* 1 2)
a
(define test (lambda () (define f _add)))
(test)
(caddr test)
(setetop '_MAKERLISP_L_ '())
(define *allow-forget* 't)
(forget '_MAKERLISP_L_)
cadr
(include '<makerlisp.l>)
(cadr '(1 2 3))
(setetop '*mark* '())
(print 'Kill this line\n ... )
(define test (lambda (x)))
(test)
(test 1)
(test 1 2)
(define *never-forget* 't)
(cadr '(1 2))
(setq *never-forget* '())
(cadr '(1 2))
(forget '*never-forget*)
(trace cdar)
(trace cadr)
(cadr '(1 2))
(untrace cadr)
(forget 'cadr)
(untrace cdar)
(include '<stdio.l>)
(~ -1)
(~ 'a)
(real2int (int2real 1))
(^ 1 -1)
'("\a\b\e\r\t\n\\\"")
'^1
(example)
'"           "
;          aaa
\\\
(reverse '(1 2 3 4))
(reverse '(1 2 3 . 4))
(reverse 'a)
(reverse (reverse (reverse '(1 2 3))))
(setq)
(define a)
(define b)
(define c)
(define test (lambda () (setq a 1 b 2 c 3)))
(test)
test
(car 1)
(cdr 1)
(zerop 'a)
