package staevtangel

import (
	"appengine"
	"appengine/user"
	
	"fmt"
	
	"net/http"
)


func init() {
	http.HandleFunc("/",monitorHandler)
}

func monitorHandler(w http.ResponseWriter, r *http.Request) {
	if !userAuthorized(w,r) {
		http.Error(w,"User not authorized",http.StatusTeapot)
		return
	}
	
	logoutURL, _ := user.LogoutURL(appengine.NewContext(r),r.URL.String())
	fmt.Fprintf(w,"<html><body>Cool info goes here.<br>Hi, %s!<br><a href=\"%v\">Logout</a></body></html>",user.Current(appengine.NewContext(r)).Email,logoutURL)
}
