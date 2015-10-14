(assert (= 1 1))
(assert (!= 1 2))
(assert (eq? 1 1))
(assert (equal? 1 1))
(assert (not (eq? 1 2)))
(assert (not (equal? 1 2)))

(define v0 (quote (1 2)))
(define v1 (quote (1 2)))
(assert (eq? v0 v0))
(assert (not (eq? v0 v1)))
(assert (equal? v0 v1))

(assert (= (+ 1 2 3) 6))
(assert (= (- 1 2 3) -4))
(assert (= (* 1 2 3) 6))
(assert (= (/ 1.0 1 2) 0.5))

(assert (= ((lambda () 1)) 1))

(define lambda-test (lambda (x y) (+ x y)))
(assert (= (lambda-test 1 2) 3))

(define (make-adder x) (lambda (a) (+ x a)))
(define add-1 (make-adder 1))
(define add-2 (make-adder 2))
(assert (= (add-1 1) 2))
(assert (= (add-2 1) 3))
(assert (= (add-1 3) 4))

(define inner-lambda-test (lambda (x) (lambda (y) (+ x y))))
(assert (= ((inner-lambda-test 1) 2) 3))

(define (expanded-let-add-4-test y)
    ((lambda (x)
         (+ x y)) 4))
(assert (= (expanded-let-add-4-test 6) 10))

(define (add-4-inside-let y)
    (let ((x 4))
        (+ x y)))
(assert (= (add-4-inside-let 6) 10))
(assert (= (add-4-inside-let 7) 11))
(assert (= (add-4-inside-let 6) 10))

(define (add x y) (+ x y))
(assert (= (apply add (list 1 2)) 3))

(define (f x y) (begin (+ x y) (+ x y)))
assert (= (f 1 2) 3))

(define variadic-test-1 (lambda values (apply + values)))
(assert (= (variadic-test-1 1 2 3) 6))

(define my-add (lambda values (apply + values)))
(define variadic-test-2 (lambda values (apply my-add values)))
(assert (= (variadic-test-2 1 2 3) 6))

(define (variadic-test-3 . values) (apply + values))
(assert (= (variadic-test-3 1 2 3) 6))

(define (variadic-test-4 x . rest) (begin (apply + rest)))
(assert (= (variadic-test-4 9 1 2 3) 6))

(define variadic-test-5 (lambda (x . rest) (begin (apply + rest))))
(assert (= (variadic-test-5 9 1 2 3) 6))

(define (make-list value count)
    (if (> count 0)
        (cons value (make-list value (- count 1)))
        (list)))
(assert (equal? (make-list 1 5) (list 1 1 1 1 1)))

(define (fact n)
    (if (<= n 1)
        1
        (* n (fact (- n 1)))))
(assert (= (fact 6) 720))

(define (expanded-let-test x) ((lambda (y) (+ x y)) 2))
(assert (= (expanded-let-test 1) 3))

(define let-test-1 (lambda (x) (let ((y 2)) (+ x y))))
(assert (= (let-test-1 1) 3))

(assert (equal? (quote (1 2 3)) (list 1 2 3)))
(assert (= (eval (quote (+ 1 2))) 3))
