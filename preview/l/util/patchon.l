; Turn on patching of VM command expansions
(define patchon
  (macro ()
    (forget 'nopatch)
   `(global "*don't-patch*" '())))
