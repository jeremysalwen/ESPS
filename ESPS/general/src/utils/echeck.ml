; @(#)echeck.ml	3.1 10/20/87 ESI
(defun (echeck $espsfunc $cmd
	    (temp-use-buffer "*echeck*"
		(erase-buffer)
		(set-mark)
		(setq $espsfunc (arg 1 "ESPS function: " ))
		(setq $cmd (concat "echeck" " " $espsfunc))
		(fast-filter-region $cmd)
		(beginning-of-file)
		(use-typeout-window "*echeck*")
	    )
	(novalue)
))

