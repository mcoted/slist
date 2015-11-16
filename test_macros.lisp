;(defmacro (for var in values body)	
;
;(for a in '(1 2 3) (println a))

;(define (f v)
;	(let ((g 0))
;		(begin
;			(set! g (lambda (x)
;						 (if (empty? x) 
;						 	'ok
;						 	(begin
;						 		(println (car x))
;							 	(g (cdr x))))))
;			(g v)))
;(f '())

(define (f)
	(let ((a 'ok))
		a))
(println (f))
