; (define (fact n)
;     (if (<= n 1)
;         1
;         (* n (fact (- n 1)))))
; 
; (println (fact 2))


(define (long-sum-2 n) ;; Tail-call optimized
    (letrec ((inner-sum (lambda (acc x) 
                (if (= x 0)
                    acc
                    (inner-sum (+ acc 1) (- x 1))))))
        (inner-sum 0 n)))

(println (long-sum-2 10000))

; (define (letrec-test v)
;     (letrec ((x (lambda (c) (if (= c 1) 1 (+ 1 (letrec-test (- c 1)))))))
;         (x v)))
; 
; (println (letrec-test 3))
