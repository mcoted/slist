(defmacro (foreach var in values body)
	'(letrec ((loop (lambda (x)
						(if (empty? x)
							'()
							(let ((,var (car x)))
								(begin
									(eval ,body)
									(loop (cdr x))))))))
		(loop ,values)))

(foreach a in '(1 2 3) (println a))

;(defmacro (run-test2 expr)
;    '(begin
;        (print "Evaluating: ")
;        (print (quote ,expr))
;        (if (eval ,expr)
;            (println "     OK")
;            (println "     FAILED"))))
;
;(run-test2 (= 1 1))