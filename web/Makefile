.PHONY: send

send:
	lftp -e 'mirror -c -R -x Makefile . public_html/angel/' -p 21 -u staevt,vehicle ftp.hutman.net
