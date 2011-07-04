22787917558746413706 1000    6 12345678901234567890    2 00000000000000000000
     181                                           doc.info          0
    7055                                               1.mm          0
SETUP_MAGIC:1
SETUP_LANDSCAPE:0
SETUP_ODD:0
SETUP_EVEN_ORG:0
SETUP_P1_ORG:0
SETUP_TOP_OFS:0
SETUP_LEFT_OFS:0
SETUP_NUMBER:70
SETUP_CLASS:Printer
SETUP_WIDTH:8500
SETUP_HEIGHT:11000
Last saved by Words in Aster*x version 2.0.360 
H0 RV131
H1 SB1 SR0 ST1 CT1 HY1 SE1 SX0 BG0 TF0 TC0 IX0 LA1 PN1 PG1 HF0 SU70 SN"Printer"\
 SC0 WN40,0,8694,9147 TW8500 TH11000 SG0 RM0 RB0 SS0 TB2 DM0 DC"." PF"" ZM0
H2
RU TB4369,"a",8738,"b",13107,"c",17476,"d",21845,"e",26214,"f",30583,"g",34952,\
"h",39321,"i",43690,"j",48059,"k",52428,"l",56797,"m",61166,"n"
RS NM2 MV1 TN"footnote" TD"FT","1","",""
WP RN20
AL LI0,0 BR0,"",65535,"a",6553,"a",65535,""
CO RN1 JU0
EF RN1
LF RN2
EF RN3
CO RN2
EF RN1
SF RN2
EF RN3
IC CP97,0,0
RE
WP RN23
AL LI0,0 BR0,"",65535,""
CO RN1
EF RN1
SE
NM RN3
RU TB4369,"a",8738,"b",13107,"c",17476,"d",21845,"e",26214,"f",30583,"g",34952,\
"h",39321,"i",43690,"j",48059,"k",52428,"l",56797,"m",61166,"n"
SE
H1 SB1 SR1 ST1 CT1 HY1 SE0 SX0 BG0 TF0 TC0 IX0 LA1 PN1 PG1 HF0 SU70 SN"Printer"\
 SC55 WN1013,440,12587,9507 TW8500 TH11000 SG0 RM0 RB0 SS0 TB2 DM0 DC"." PF"" Z\
M2
H2
H3 ON0
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO JU1
AT SZ9
EF
TXUpdate to ESPS/
TXwaves+
AT IT1
TX Installation Instructions 
WP
AL LI0,0 BR0,"",65535,""
CO JU1
AT SZ9
EF
TXfor 
TX 
AT IT1
TX5.0
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO JU1
EF
TX11 August 1993
WP
AL LI0,0 BR0,"",65535,""
CO JU0
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXThis document updates the enclosed ESPS/
TXwaves+
AT IT1
TX installation instructions.    There are no significant differences in the in\
stallation procedure between this release and the release described in the inst\
ructions. 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXIf you are upgrading from a previous version of ESPS/
TXwaves+
AT IT1
TX it is particularly important that you read the enclosed release notes.  Also\
, if you are installing this product, but you are not the primary user, please \
see that the users get the release notes and the tutorials.   The release notes\
 - along with all other documentation, are available online via the programs 
TXeinfo
AT IT1
TX or 
TXeversion
AT IT1
TX.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXAlso note that if you are upgrading from ESPS/
AT BL1
TXwaves+ 
AT BL1 IT1
TX4.1/2.1 or 4.2/3.1, then your license key files are the same.  You can save y\
ourself (and us) some trouble by saving the contents of the old 
AT BL1
TX$ESPS_BASE/lib/keys
AT BL1 IT1
TX directory before you remove the old ESPS and copy those files into the new 
AT BL1
TX$ESPS_BASE/lib/keys
AT BL1 IT1
TX directory.
AT BL1
TX   Be sure that you stop the old 
TXelmd.
AT IT1
TX  If 
TXelmd
AT IT1
TX is being started in the 
TX/etc/rc
AT IT1
TX file be sure that you update the pathname in there.  If you are updating fro\
m an ESPS release prior to 4.1, then the old key files will  not work and you w\
ill need to get new keys from us. 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXThe differences are:
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 6 (System Requirements): 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe Sparc release requires Sun OS 4.1.1 or greater or Solaris 2.1 or greate\
r.  The IRIS release \t\t\trequires IRIS 4.0.5F or greater.  The HP700 release \
requires HPUX 9.01 or later.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 8 (Size of Distribution):
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe ESPS/
TXwaves
AT IT1
TX+ distributions  require about 110 megabytes of disk space.  The 
AT
TXwaves
AT IT1
TX+ only \t\t\tversion can be installed in about 50 megabytes.    
AT
IC CP97,0,95
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 13 (Name of Tape Device)
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tFor 4mm (DAT) tapes on SGI machines , if you have trouble using the normal \
tape device, try the \t\tbyte-swap entry to the driver by adding the characters\
 
TXns
AT BL1
TX after the device name.  For example, \t\t\t/dev/rmt/tps0d2ns.  
AT
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 16 (License Manager)
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe programs 
TXwcheckout
AT IT1
TX and 
AT
TXwfree
AT IT1
TX are no longer needed and have been removed from the distribution.    
AT
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 24 (License Manager)
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe startup time for the license server has been reduced to 1 minutes, from\
 3 minutes.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 27 (License Manager)
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe example given on this page cannot occur.  
TXtestsd
AT IT1
TX, along with a core set of other programs will run if \ta site has a valid 
TXwaves+
AT IT1
TX license on the network.  It does not matter if an ESPS license is checked ou\
t, 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tor if 
TXxwaves
AT IT1
TX is being run.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TXPage 31 (Hardcopy Plotting)
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThis version of the system is configured so that if the 
TX-T hardcopy
AT BL1
TX option is used on the plot programs, \tthe default action is to send Postscr\
ipt to the spooler device named 
TXlpr
AT IT1
TX.   This is correct for 90% of the \t\tUnix systems out there.    If you have\
 an HP LaserJet printer use the 
TX-Thp
AT BL1
TX option on the programs 
TXaplot,
AT IT1
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tgenplot, mlplot, plotsd, plotspec, 
AT IT1
TXand
TX scatplot.
AT IT1
TX   Pipe the standard output of these programs to your spool
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tdevice.   (For example, 
TXplotsd -Thp file.sd | lpr
AT BL1
TX).   To use 
TXplotsgram
AT IT1
TX with an HP LaserJet, simply use 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tthe 
TX-Thp
AT BL1
TX option.  The program will spool the output to 
TXlpr
AT IT1
TX by default.  If this is not the name of your 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tspool program, then edit the file 
TXbin/plotsgram
AT IT1
TX, and change the assignment to 
TXHP_SPOOL_CMD
AT BL1
TX near
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tthe top of the file.   Note that you can set up different spool commands fo\
r HP and for Postscript.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tThe installation manual implies that 
TXtek2ps
AT IT1
TX is not already compiled and installed into the system.  This is \tnot correc\
t.  It is installed.   For other laser printers other than Postscript for HP co\
mpatible, please 
WP
AL LI0,0 BR0,"",65535,""
CO
EF
TX\tcontact us.   The only other type directly supported is Imagen.
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
WP
AL LI0,0 BR0,"",65535,""
CO
EF
SE
RS NM3
WP IW256
AL LI0,0 BR0,"",65535,""
CO JU1
AT BL0 UL0 IT0 SS0 SZ5 FO4 HY1 CO1
EF
TXFootnotes
RE
SE
