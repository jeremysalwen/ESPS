;; Functions eman and eman-k to read ESPS man pages and perform
;; keyword search.  Also echeck to search ESPS lint library.
;; By John Shore modifying man.el from GNU

;; Copyright (C) 1985, 1986 Free Software Foundation, Inc.

;; This file is part of GNU Emacs.
;; @(#)esps.el	1.1 2/21/89 ESI GNU 

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY.  No author or distributor
;; accepts responsibility to anyone for the consequences of using it
;; or for whether it serves any particular purpose or works at all,
;; unless he says so in writing.  Refer to the GNU Emacs General Public
;; License for full details.

;; Everyone is granted permission to copy, modify and redistribute
;; GNU Emacs, but only under the conditions described in the
;; GNU Emacs General Public License.   A copy of this license is
;; supposed to have been given to you along with GNU Emacs so you
;; can know your rights and responsibilities.  It should be in a
;; file named COPYING.  Among other things, the copyright notice
;; and this notice must be preserved on all copies.

(load "tags")
(defvar eman-program "eman")
(defvar k-option "-k")
(defvar echeck-program "echeck")

(defun eman (topic &optional section)
  "Display the ESPS manual entry for TOPIC.
TOPIC is either the title of the entry, or has the form TITLE(SECTION)
where SECTION is the desired section of the manual, as in `classify(3)'."
  (interactive (if current-prefix-arg
		   '(nil t)
		 (find-tag-tag "Program or Program(section): ")))
  (if (and (null section)
	   (string-match "\\`[ \t]*\\([^( \t]+\\)[ \t]*(\\(.+\\))[ \t]*\\'" topic))
      (setq section (substring topic (match-beginning 2)
				     (match-end 2))
	    topic (substring topic (match-beginning 1)
				   (match-end 1))))
  (with-output-to-temp-buffer "*Eman Entry*"
    (buffer-flush-undo standard-output)
    (save-excursion
      (set-buffer standard-output)
      (if (= (buffer-size) 0)
	  (progn
	    (message "Invoking eman %s%s..."
		     (if section (concat section " ") "") topic)
	    (if section
		(call-process eman-program nil t nil section topic)
	        (call-process eman-program nil t nil topic))
	    (if (< (buffer-size) 80)
		(progn
		  (goto-char (point-min))
		  (end-of-line)
		  (error (buffer-substring 1 (point)))))))

      (message "Cleaning manual entry for %s..." topic)
      (nuke-nroff-bs)
      (set-buffer-modified-p nil)
      (message ""))))

;; Hint: BS stands form more things than "back space"
(defun nuke-nroff-bs ()
  (interactive "*")
  ;; Nuke underlining and overstriking (only by the same letter)
  (goto-char (point-min))
  (while (search-forward "\b" nil t)
    (let* ((preceding (char-after (- (point) 2)))
	   (following (following-char)))
      (cond ((= preceding following)
	     ;; x\bx
	     (delete-char -2))
	    ((= preceding ?\_)
	     ;; _\b
	     (delete-char -2))
	    ((= following ?\_)
	     ;; \b_
	     (delete-region (1- (point)) (1+ (point)))))))

  ;; Nuke headers: "MORE(1) UNIX Programmer's Manual MORE(1)"
  (goto-char (point-min))
  (while (re-search-forward "^ *\\([A-Za-z][-_A-Za-z0-9]*([0-9A-Z]+)\\).*\\1$" nil t)
    (replace-match ""))
  
  ;; Nuke footers: "Printed 12/3/85	27 April 1981	1"
  ;;    Sun appear to be on drugz:
  ;;     "Sun Release 3.0B  Last change: 1 February 1985     1"
  ;;    HP are even worse!
  ;;     "     Hewlett-Packard   -1- (printed 12/31/99)"  FMHWA12ID!!
  ;;    System V (well WICATs anyway):
  ;;     "Page 1			  (printed 7/24/85)"
  ;;    Who is administering PCP to these corporate bozos?
  (goto-char (point-min))

;;should modify the following commented-out code for ESPS:

;  (while (re-search-forward
;	   (cond ((eq system-type 'hpux)
;		  "^[ \t]*Hewlett-Packard\\(\\| Company\\)[ \t]*- [0-9]* -.*$")
;		 ((eq system-type 'usg-unix-v)
;		  "^ *Page [0-9]*.*(printed [0-9/]*)$")
;		 (t
;		  "^\\(Printed\\|Sun Release\\) [0-9].*[0-9]$"))
;	   nil t)
;    (replace-match ""))

  ;; Crunch blank lines
  (goto-char (point-min))
  (while (re-search-forward "\n\n\n\n*" nil t)
    (replace-match "\n\n"))

  ;; Nuke blanks lines at start.
  (goto-char (point-min))
  (skip-chars-forward "\n")
  (delete-region (point-min) (point)))

(defun eman-k (keyword &optional section)
  "Display ESPS programs matching keyword."
;; leave code in place to handle section since eman -k should
  (interactive "sKeyword: ")
  (if (and (null section)
	   (string-match "\\`[ \t]*\\([^( \t]+\\)[ \t]*(\\(.+\\))[ \t]*\\'" keyword))
      (setq section (substring keyword (match-beginning 2)
				     (match-end 2))
	    keyword (substring keyword (match-beginning 1)
				   (match-end 1))))
  (with-output-to-temp-buffer "*eman-k output*"
    (buffer-flush-undo standard-output)
    (save-excursion
      (set-buffer standard-output)
      (if (= (buffer-size) 0)
	  (progn
	    (message "Invoking eman -k %s%s..."
		     (if section (concat section " ") "") keyword)
	    (if section
		(call-process eman-program nil t nil k-option section keyword)
	        (call-process eman-program nil t nil k-option keyword))
	    (if (< (buffer-size) 80)
		(progn
		  (goto-char (point-min))
		  (end-of-line)
		  (error (buffer-substring 1 (point)))))))
      (set-buffer-modified-p nil)
      (message ""))))


(defun echeck (program &optional section)
  "Display ESPS function synopsis."
  (interactive (if current-prefix-arg
		   '(nil t)
		 (find-tag-tag "Function: ")))
  (if (and (null section)
	   (string-match "\\`[ \t]*\\([^( \t]+\\)[ \t]*(\\(.+\\))[ \t]*\\'" program))
      (setq section (substring program (match-beginning 2)
				     (match-end 2))
	    program (substring program (match-beginning 1)
				   (match-end 1))))
  (with-output-to-temp-buffer "*echeck output*"
    (buffer-flush-undo standard-output)
    (save-excursion
      (set-buffer standard-output)
      (if (= (buffer-size) 0)
	  (progn
	    (message "Invoking eheck %s%s..."
		     (if section (concat section " ") "") program)
	    (if section
		(call-process echeck-program nil t nil section program)
	        (call-process echeck-program nil t nil program))
	    (if (< (buffer-size) 80)
		(progn
		  (goto-char (point-min))
		  (end-of-line)
		  (error (buffer-substring 1 (point)))))))
      (set-buffer-modified-p nil)
      (message ""))))





    
