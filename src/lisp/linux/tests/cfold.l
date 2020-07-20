; Test folding of constant integer arithmetic expressions

; First, just one at a time

; 1 + 1 = 2
(define cf_add (lambda () (+ 1 1)))
(cf_add)
cf_add

; -2 >> 1 = -1
(define cf_asr (lambda () (>> -2 1)))
(cf_asr)
cf_asr

; -2 / 2 = -1
(define cf_div (lambda () (/ -2 2)))
(cf_div)
cf_div

; 1 / 0 = ?
(define cf_div (lambda () (/ 1 0)))
(cf_div)
cf_div

; -1 & -2 = -2
(define cf_iand (lambda () (& -1 -2)))
(cf_iand)
cf_iand

; 99 % 10 = 9
(define cf_imod (lambda () (% 99 10)))
(cf_imod)
cf_imod

; 99 % 0 = ?
(define cf_imod (lambda () (% 99 0)))
(cf_imod)
cf_imod

; ~ 0 = -1 
(define cf_inot (lambda () (~ 0)))
(cf_inot)
cf_inot

; 1 | 2 = 3 
(define cf_ior (lambda () ("|" 1 2)))
(cf_ior)
cf_ior

; -1 ^ -2 = 1 
(define cf_ixor (lambda () (^ -1 -2)))
(cf_ixor)
cf_ixor

; 3 * 4 = 12 
(define cf_mul (lambda () (* 3 4)))
(cf_mul)
cf_mul

; -1 << 1 = -2 
(define cf_shl (lambda () (<< -1 1)))
(cf_shl)
cf_shl

; 1 - -1 = 2 
(define cf_sub (lambda () (- 1 -1)))
(cf_sub)
cf_sub

; Then, in combination

; -2
(define cf_shl (lambda () (<< (~ 0) 1)))
(cf_shl)
cf_shl

; -1
(define cf_asr (lambda () (>> (<< (~ 0) 1) 1)))
(cf_asr)
cf_asr

; -2
(define cf_sub (lambda () (- (>> (<< (~ 0) 1) 1) 1)))
(cf_sub)
cf_sub

; 0x7fffff
(define test (lambda () (& (>> (<< (~ 0) 1) 1) (- (<< 1 23) 1))))
(sprintf '"%#x" (test))
test

; 0x7fffff
(define test (lambda () (& (>> (<< (~ 0) 1) 1) (_sub (<< 1 23) 1))))
(sprintf '"%#x" (test))
test

; 0x7fffff
(define cf_iand (macro () (& (>> (<< (~ 0) 1) 1) (- (<< 1 23) 1))))
(define test (lambda () (cf_iand)))
(sprintf '"%#x" (test))
test
