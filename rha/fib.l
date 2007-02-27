(defun fib (n) (if (< n 3) 1 (+ (fib (- n 1)) (fib (- n 2)))))

(defun hermite (n x) 
  (if (= n 0) 
      1 
    (if (= n 1) 
	x 
      (- (* x (hermite (- n 1) x)) (hermite (- n 2) x)))))