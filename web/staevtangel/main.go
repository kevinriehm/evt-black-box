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
}

func monitorHandler(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	logoutURL, _ := user.LogoutURL(appengine.NewContext(r),r.URL.String())
	
	if !userAuthorized(c,user.Current(c)) {
		http.Error(w,"User not authorized\n"+logoutURL,http.StatusForbidden)
		return
	}
	
	fmt.Fprintf(w,"<html><body>Cool info goes here.<br>Hi, %s!<br><a href=\"%v\">Logout</a></body></html>",user.Current(appengine.NewContext(r)).Email,logoutURL)
}
