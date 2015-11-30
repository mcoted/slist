;(defmacro (foreach var in values body)
;	'(letrec ((loop (lambda (x)
;						(if (empty? x)
;							'()
;							(let ((,var (car x)))
;								(begin
;									(eval ,body)
;									(loop (cdr x))))))))
;		(loop ,values)))

;(foreach a in '(1 2 3) (println a))

;(define (process-files)
;	(foreach filename in '("test1.txt" "test2.txt" "test3.txt")
;		(begin
;			(println filename))))
;(process-files)

(defmacro (run-test2 expr)
    '(begin
        (print "Evaluating: ")
        (print (quote ,expr))
        (if (eval ,expr)
            (println "     OK")
            (println "     FAILED"))))

(run-test2 (= 1 1))