package staevtangel

import (
	"appengine"
	"appengine/user"
	
	"net/http"
	
	"regexp"
)


type AccessException struct {
	Email		string
	Authorized	bool
}


func userAuthorized(w http.ResponseWriter, r *http.Request) bool {
	c := appengine.NewContext(r)
	u := user.Current(c)
	
	// Check for an exception first
	
	// Otherwise, default to checking the domain
	matched, _ := regexp.MatchString("@cadets.com$",u.Email)
	
	return matched
}
