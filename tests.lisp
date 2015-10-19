(___run_test (= 1 1))
(___run_test (!= 1 2))
(___run_test (eq? 1 1))
(___run_test (equal? 1 1))
(___run_test (not (eq? 1 2)))
(___run_test (not (equal? 1 2)))

(define v0 (quote (1 2)))
(define v1 (quote (1 2)))
(___run_test (eq? v0 v0))
(___run_test (not (eq? v0 v1)))
(___run_test (equal? v0 v1))

(___run_test (= (+ 1 2 3) 6))
(___run_test (= (- 1 2 3) -4))
(___run_test (= (* 1 2 3) 6))
(___run_test (= (/ 1.0 1 2) 0.5))

(___run_test (= ((lambda () 1)) 1))

(define lambda-test (lambda (x y) (+ x y)))
(___run_test (= (lambda-test 1 2) 3))

(define (make-adder x) (lambda (a) (+ x a)))
(define add-1 (make-adder 1))
(define add-2 (make-adder 2))
(___run_test (= (add-1 1) 2))
(___run_test (= (add-2 1) 3))
(___run_test (= (add-1 3) 4))

(define inner-lambda-test (lambda (x) (lambda (y) (+ x y))))
(___run_test (= ((inner-lambda-test 1) 2) 3))

(define (expanded-let-add-4-test y)
    ((lambda (x)
         (+ x y)) 4))
(___run_test (= (expanded-let-add-4-test 6) 10))

(define (add-4-inside-let y)
    (let ((x 4))
        (+ x y)))
(___run_test (= (add-4-inside-let 6) 10))
(___run_test (= (add-4-inside-let 7) 11))
(___run_test (= (add-4-inside-let 6) 10))

(define (add x y) (+ x y))
(___run_test (= (apply add (list 1 2)) 3))

(define (f x y) (begin (+ x y) (+ x y)))
(___run_test (= (f 1 2) 3))

(define variadic-test-1 (lambda values (apply + values)))
(___run_test (= (variadic-test-1 1 2 3) 6))

(define my-add (lambda values (apply + values)))
(define variadic-test-2 (lambda values (apply my-add values)))
(___run_test (= (variadic-test-2 1 2 3) 6))

(define (variadic-test-3 . values) (apply + values))
(___run_test (= (variadic-test-3 1 2 3) 6))

(define (variadic-test-4 x . rest) (begin (apply + rest)))
(___run_test (= (variadic-test-4 9 1 2 3) 6))

(define variadic-test-5 (lambda (x . rest) (begin (apply + rest))))
(___run_test (= (variadic-test-5 9 1 2 3) 6))

(define (make-list value count)
    (if (> count 0)
        (cons value (make-list value (- count 1)))
        (list)))
(___run_test (equal? (make-list 1 5) (list 1 1 1 1 1)))

(define (fact n)
    (if (<= n 1)
        1
        (* n (fact (- n 1)))))
(___run_test (= (fact 6) 720))

(define (expanded-let-test x) ((lambda (y) (+ x y)) 2))
(___run_test (= (expanded-let-test 1) 3))

(define let-test-1 (lambda (x) (let ((y 2)) (+ x y))))
(___run_test (= (let-test-1 1) 3))

(___run_test (equal? (quote (1 2 3)) (list 1 2 3)))
(___run_test (= (eval (quote (+ 1 2))) 3))

(define (long-sum n)
    (if (= n 1)
        1
        (+ 1 (long-sum (- n 1)))))
(___run_test (= (long-sum 100) 100))
;(___run_test (= (long-sum 1000) 1000)) ;; TODO: Need tail-recursive-call optimization

(___run_test (eq? 'a 'a))

(define symb-symb ''a)
(___run_test (not (eq? 'a symb-symb')))
(___run_test (equal? 'a symb-symb))

