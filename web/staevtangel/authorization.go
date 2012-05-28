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

func ensureAuthorization(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	if !userAuthorized(c,user.Current(c)) {
		url, _ := user.LoginURL(c,r.URL.String())
		w.Header().Set("Location",url)
		w.WriteHeader(http.StatusFound)
		return
	}
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
	iter := datastore.NewQuery("AccessException").Run(c)
	for {
		var ex AccessException
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

func addAccessException(c appengine.Context, email string, authorized bool) (oldKeyStr string, newKey *datastore.Key) {
	// Remove the previous exception, if it exists and is different
	iter := datastore.NewQuery("AccessException").Run(c)
	for {
		var ex AccessException
		oldKey, err := iter.Next(&ex);
		
		if err == datastore.Done {
			break
		}
		
		if ex.Email == email {
			if ex.Authorized == authorized {
				return "", nil
			} else {
				oldKeyStr = oldKey.Encode()
				datastore.Delete(c,oldKey)
				break
			}
		}
	}
	
	// Add the new exception
	ex := AccessException{
		Email:		email,
		Authorized:	authorized,
	}
	newKey, _ = datastore.Put(c,datastore.NewIncompleteKey(c,"AccessException",nil),&ex)
	
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
