(defmacro (for var in values body)
	'(letrec ((loop (lambda (x)
						(if (empty? x)
							'()
							(let ((,var (car x)))
								(begin
									(eval ,body)
									(loop (cdr x))))))))
		(loop ,values)))

;(for a in '(1 2 3) (println a))

(define (process-files)
	(for filename in '("test1.txt" "test2.txt" "test3.txt")
		(begin
			(println filename))))
(process-files)