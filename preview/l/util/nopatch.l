; Turn off patching of VM command expansions
(define nopatch
  (macro ()
    (forget 'nopatch)
   `(global "*don't-patch*" 't)))
