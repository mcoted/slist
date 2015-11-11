;; Basics
(run-test (= 1 1))
(run-test (!= 1 2))
(run-test (eq? 1 1))
(run-test (equal? 1 1))
(run-test (not (eq? 1 2)))
(run-test (not (equal? 1 2)))

;; Predicates
(run-test (pair? '(1 2)))
(run-test (boolean? (eq? 1 1)))
(run-test (integer? 1))
(run-test (number? 1.1))
(run-test (string? "hello"))
(run-test (symbol? 'a))

;; Equals
(define v0 '(1 2))
(define v1 '(1 2))
(run-test (eq? v0 v0))
(run-test (not (eq? v0 v1)))
(run-test (equal? v0 v1))

(run-test (equal? (list 1 2 3) (list 1 2 3)))
(run-test (not (equal? (list 1 2 3) (list 1 2 4))))

(run-test (= (+ 1 2 3) 6))
(run-test (= (- 1 2 3) -4))
(run-test (= (* 1 2 3) 6))
(run-test (= (/ 1.0 1 2) 0.5))

;; Defines
(define (f)
    (begin
        (define x 5)
        x))
(run-test (= (f) 5))

;; Lambdas
(run-test (= ((lambda () 1)) 1))

(define lambda-test (lambda (x y) (+ x y)))
(run-test (= (lambda-test 1 2) 3))

(define (make-adder x) (lambda (a) (+ x a)))
(define add-1 (make-adder 1))
(define add-2 (make-adder 2))
(run-test (= (add-1 1) 2))
(run-test (= (add-2 1) 3))
(run-test (= (add-1 3) 4))

(define inner-lambda-test (lambda (x) (lambda (y) (+ x y))))
(run-test (= ((inner-lambda-test 1) 2) 3))

(define (expanded-let-add-4-test y)
    ((lambda (x)
         (+ x y)) 4))
(run-test (= (expanded-let-add-4-test 6) 10))

(define (add-4-inside-let y)
    (let ((x 4))
        (+ x y)))
(run-test (= (add-4-inside-let 6) 10))
(run-test (= (add-4-inside-let 7) 11))
(run-test (= (add-4-inside-let 6) 10))

(define (add x y) (+ x y))
(run-test (= (apply add '(1 2)) 3))

(define (f x y) (begin (+ x y) (+ x y)))
(run-test (= (f 1 2) 3))

;; Variadic arguments
(define variadic-test-1 (lambda values (apply + values)))
(run-test (= (variadic-test-1 1 2 3) 6))

(define my-add (lambda values (apply + values)))
(define variadic-test-2 (lambda values (apply my-add values)))
(run-test (= (variadic-test-2 1 2 3) 6))

(define (variadic-test-3 . values) (apply + values))
(run-test (= (variadic-test-3 1 2 3) 6))

(define (variadic-test-4 x . rest) (begin (apply + rest)))
(run-test (= (variadic-test-4 9 1 2 3) 6))

(define variadic-test-5 (lambda (x . rest) (begin (apply + rest))))
(run-test (= (variadic-test-5 9 1 2 3) 6))

(define (variadic-test-6 x . rest) (car rest))
(run-test (= (variadic-test-6 1 (+ 2 3)) 5))

(define (make-list value count)
    (if (> count 0)
        (cons value (make-list value (- count 1)))
        (list)))
(run-test (equal? (make-list 1 5) '(1 1 1 1 1)))

(define (fact n)
    (if (<= n 1)
        1
        (* n (fact (- n 1)))))
(run-test (= (fact 6) 720))

;; Let
(define (expanded-let-test x) ((lambda (y) (+ x y)) 2))
(run-test (= (expanded-let-test 1) 3))

(define let-test-1 (lambda (x) (let ((y 2)) (+ x y))))
(run-test (= (let-test-1 1) 3))

(run-test (equal? (quote (1 2 3)) (list 1 2 3)))
(run-test (= (eval (quote (+ 1 2))) 3))

;; Recursion
(define (long-sum n)
    (if (= n 1)
        1
        (+ 1 (long-sum (- n 1)))))
(run-test (= (long-sum 100) 100))
;(run-test (= (long-sum 1000) 1000)) ;; TODO: Need tail-call optimization

;; Symbols
(run-test (eq? 'a 'a))
(run-test (eq? 'a (quote a)))

(define quote-a ''a)
(run-test (not (eq? ''a quote-a)))

(define one 1)
(run-test (eq? (quote one) 'one))
(run-test (= (quote (unquote one)) 1))
(run-test (= (quote ,one) 1))
(run-test (equal? (list 1 2 3) '(,one 2 3)))
(run-test (equal? '((1 2)) '((,one 2))))

(define some-string "hello")
(run-test (equal? some-string "hello"))

(define set-value 1)
(set! set-value 2)
(run-test (= set-value 2))
(set! set-value (+ set-value 1))
(run-test (= set-value 3))

(run-test (= (apply (eval (car '(+ 1 2))) '(3 4)) 7))

;; Macros
(defmacro (adder-macro x y) '(+ ,x ,y))
(run-test (= (adder-macro 1 2) 3))



