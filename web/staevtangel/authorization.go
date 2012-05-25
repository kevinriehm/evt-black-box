package staevtangel

import (
	"appengine"
	"appengine/datastore"
	"appengine/user"
	
	"net/http"
	
	"regexp"
)

type AccessException struct {
	Email		string
	Authorized	bool
}

func userAuthorized(w http.ResponseWriter, r *http.Request) bool {
	var ex AccessException
	
	c := appengine.NewContext(r)
	u := user.Current(c)
	
	// Allow in administrators, of course
	if user.IsAdmin(c) {
		return true
	}
	
	// Check for an exception
	iter := datastore.NewQuery("AccessException").Run(c)
	for {
		_, err := iter.Next(&ex);
		if err == datastore.Done {
			break
		}
		if ex.Email == u.Email {
			return ex.Authorized
		}
	}
	
	// Default to checking the domain
	matched, _ := regexp.MatchString("@cadets.com$",u.Email)
	
	return matched
}
