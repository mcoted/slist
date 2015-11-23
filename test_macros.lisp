;(defmacro (for var in values body)	
;
;(for a in '(1 2 3) (println a))

;(define (f v)
;	(let ((g (lambda (x) (if (= x 0) 0 (g (- x 1))))))
;		(g v)))
;(f 1)

;(define (f v)
;	((lambda (g) (begin (set! g g-value) (g v)))
;		(lambda (x) (if (= x 0) 0 (g (- x 1))))))
;(f 1)

; (define (f)
; 	((lambda (x) (x))
; 		(lambda () (x))))
;
;(define (f)
;	(let ((x (lambda () (x))))
;		(x)))
;
;;;;;
;
;(define (f)
;	((lambda (x) (x)) 
;		((lambda () (x)))))
;
;;; -->
;
;(define (f)
;	((lambda (x) (set! x (some-lambda (x))) (x))
;		0))
;
;;;;;
;
;(define (f)
;	((lambda (x) '())
;		1))
;
(define (f v)
	(letrec ((x (lambda (c) (if (= c 1) 1 (+ 1 (f (- c 1)))))))
		(x v)))
