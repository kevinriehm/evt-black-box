package staevtangel

import (
	"appengine"
	"appengine/datastore"
	"appengine/user"
	
	"net/http"
	"regexp"
)

type AccessException struct {
	Authorized	bool
}

func ensureAuthorization(w http.ResponseWriter, r *http.Request) bool {
	c := appengine.NewContext(r)
	if !userAuthorized(c,user.Current(c)) {
		url, _ := user.LoginURL(c,r.URL.String())
		w.Header().Set("Location",url)
		w.WriteHeader(http.StatusFound)
		return false
	}
	return true
}

func userAuthorized(c appengine.Context, u *user.User) bool {
	// Anonymous users, depart!
	if u == nil {
		return false
	}
	
	// Allow administrators, of course
	if user.IsAdmin(c) {
		return true
	}
	
	// Check for an exception
	var ex AccessException
	k := datastore.NewKey(c,"AccessException",u.Email,0,nil)
	err := datastore.Get(c,k,&ex)
	if err == nil {
		return ex.Authorized
	}
	
	// Default to checking the domain
	matched, _ := regexp.MatchString("@cadets.com$",u.Email)
	
	return matched
}

func addAccessException(c appengine.Context, email string, authorized bool) (oldKeyStr string, newKey *datastore.Key) {
	var ex AccessException
	
	// Remove the previous exception, if it exists and is different
	oldKey := datastore.NewKey(c,"AccessException",email,0,nil)
	err := datastore.Get(c,oldKey,&ex)
	if err == nil {
		if ex.Authorized == authorized {
			return "", nil
		} else {
			oldKeyStr = oldKey.Encode()
			datastore.Delete(c,oldKey)
		}
	}
	
	// Add the new exception
	ex = AccessException{
		Authorized:	authorized,
	}
	newKey, _ = datastore.Put(c,datastore.NewKey(c,"AccessException",email,0,nil),&ex)
	
	return
}

func deleteAccessException(c appengine.Context, key string) string {
	if k, err := datastore.DecodeKey(key); err == nil {
		kStr := k.Encode()
		datastore.Delete(c,k)
		return kStr
	}
	return ""
}
