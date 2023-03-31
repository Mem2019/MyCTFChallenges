; function chall_dot: perform customed dot product.
; When i == 2, it basically performs (a + [0, 1, 3]).dot(b - [18, 15, 13]) + 7.
; We should note that in lambda calculus there is no negative number, 
; so if a <= b, (- a b) is always zero.
(Y (lambda (rec a b i)
	(IF (ISEMPTY a)
		7
		(+
			(* (+ (FST a) (- (* 2 i) 5)) (- (FST b) (+ 13 (- 11 (* 3 i)))))
			(rec (SND a) (SND b) (SUCC i))))))

; function chall_mat: matrix multiply
(Y (lambda (rec m x)
	(IF (ISEMPTY m)
		NIL
		(PAIR (chall_dot (FST m) x 2) (rec (SND m) x)))))

; function chall_cmp: compare 2 lists, return number of equal element pairs
(Y (lambda (rec a b)
	(IF (ISEMPTY a)
		0
		(+ (IF (= (FST a) (FST b)) 1 0) (rec (SND a) (SND b))))))

; chall
(= 3 (chall_cmp (chall_mat matrix input) res))