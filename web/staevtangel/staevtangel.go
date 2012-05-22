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
	checkUser(w,r)
	
	fmt.Fprint(w,"Cool info goes here.")
}

// Log-in is required for basically the entire site
func checkUser(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	
	u := user.Current(c)
	if u == nil {
		url, err := user.LoginURL(c, r.URL.String())
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Location", url)
		w.WriteHeader(http.StatusFound)
		return
	}
}
