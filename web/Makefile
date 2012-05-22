SDKROOT   = $(HOME)/google_appengine
APPCFG    = $(SDKROOT)/appcfg.py
APPSERVER = $(SDKROOT)/dev_appserver.py

.PHONY: send run

# Upload to Google AppEngine
send:
	$(APPCFG) update .

# Test locally
run:
	$(APPSERVER) .
