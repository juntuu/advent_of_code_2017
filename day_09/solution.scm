(define removed 0)

(define (garbage)
  (let ((c (read-char)))
    (cond
      ((eq? c #\>) #f)
      ((eq? c #\!)
       (begin
         (read-char)
         (garbage)))
      (else
        (begin
          (set! removed (+ 1 removed))
          (garbage))))))

(define (groups out)
  (let ((c (peek-char)))
    (cond
      ((eof-object? c) out)
      ((eq? c #\newline) out)
      ((eq? c #\}) (begin (read-char) out))
      ((eq? c #\{)
       (groups (cons (group) out)))
      ((eq? c #\<)
       (begin (read-char) (garbage) (groups out)))
      ((eq? c #\,)
       (begin (read-char) (groups out))))))

(define (group)
  (begin
    (read-char)
    (groups '())))

(define (score groups unit)
  (+ unit (fold-left (lambda (acc g) (+ acc (score g (+ 1 unit)))) 0 groups)))

(display (score (group) 1))
(newline)
(display removed)
(newline)
