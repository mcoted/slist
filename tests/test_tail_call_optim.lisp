
; (define (long-sum-2 n) ;; Tail-call optimized
;     (letrec ((inner-sum (lambda (acc x) 
;                 (if (= x 0)
;                     acc
;                     (inner-sum (+ acc 1) (- x 1))))))
;         (inner-sum 0 n)))
; 
; (println (long-sum-2 10000))

(define (forever x)
	(begin
		(println x)
		(forever (+ x 1))))

(forever 0)
