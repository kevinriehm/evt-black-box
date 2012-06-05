package staevtangel

import (
	"appengine"
	"appengine/user"
	
	"fmt"
	"net/http"
)

func init() {
	http.HandleFunc("/",monitorHandler)
	http.HandleFunc("/admin",adminHandler)
	http.HandleFunc("/data",dataHandler)
}

func monitorHandler(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	
	if !userAuthorized(c,user.Current(c)) {
		redirectToLogin(w,r)
		return
	}
	
	logoutURL, _ := user.LogoutURL(appengine.NewContext(r),r.URL.String())
	fmt.Fprintf(w,"<html><body>Cool info goes here.<br>Hi, %s!<br><a href=\"%v\">Logout</a></body></html>",user.Current(appengine.NewContext(r)).Email,logoutURL)
}
