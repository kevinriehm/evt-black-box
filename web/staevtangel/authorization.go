package staevtangel

import (
	"appengine"
	"appengine/datastore"
	"appengine/user"
	
	"regexp"
)

type accessException struct {
	Email		string
	Authorized	bool
}

func userAuthorized(c appengine.Context, u *user.User) bool {
	// Allow administrators, of course
	if user.IsAdmin(c) {
		return true
	}
	
	// Check for an exception
	iter := datastore.NewQuery("accessException").Run(c)
	for {
		var ex accessException
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

func addAccessException(c appengine.Context, email string, authorized bool) (oldKey *datastore.Key, newKey *datastore.Key) {
	// Remove the previous exception, if it exists and is different
	iter := datastore.NewQuery("accessException").Run(c)
	for {
		var ex accessException
		oldKey, err := iter.Next(&ex);
		
		if err == datastore.Done {
			break
		}
		
		if ex.Email == email {
			if ex.Authorized == authorized {
				return nil, nil
			} else {
				datastore.Delete(c,oldKey)
				break
			}
		}
	}
	
	ex := accessException{
		Email:		email,
		Authorized:	authorized,
	}
	newKey, _ = datastore.Put(c,datastore.NewIncompleteKey(c,"accessException",nil),&ex)
	
	return
}

func deleteAccessException(c appengine.Context, email string) *datastore.Key {
	iter := datastore.NewQuery("accessException").Run(c)
	for {
		var ex accessException
		key, err := iter.Next(&ex);
		
		if err == datastore.Done {
			break
		}
		
		if ex.Email == email {
			datastore.Delete(c,key)
			return key
		}
	}
	return nil
}
