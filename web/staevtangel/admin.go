package staevtangel

import (
	"appengine"
	"encoding/json"
	"html/template"
	"net/http"
)

type AdminAJAXResponse struct {
	Deleted []string `json:"deleted,omitempty"`
	Added   []string `json:"added,omitempty"`
}

func adminHandler(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	
	if r.Method == "GET" { // Regular access
		
	} else if r.Method == "POST" { // AJAX update
		v := r.URL.Query()
		switch(v["action"][0]) {
			case "add_exception":
				var response AdminAJAXResponse
				oldKey, newKey := addAccessException(c,v["email"][0],v["authorized"][0] == "true")
				if oldKey != nil {
					response.Deleted = []string{oldKey.Encode()}
				}
				if newKey != nil {
					response.Added = []string{newKey.Encode()}
				}
				responseJSON, _ := json.Marshal(&response)
				w.Header().Set("Content-Type","application/json")
				w.Write(responseJSON)
				
			case "delete_exception":
				var response AdminAJAXResponse
				oldKey := deleteAccessException(c,v["email"][0])
				if oldKey != nil {
					response.Deleted = []string{oldKey.Encode()}
				}
				responseJSON, _ := json.Marshal(&response)
				w.Header().Set("Content-Type","application/json")
				w.Write(responseJSON)
		}
	}
}

var adminTemplate = `
<!doctype html>
<html>
	<head>
		<title>Angel Admin Page</title>
		
		<link rel="stylesheet" type="text/css" href="/css/admin.css">
		
		<script src="/js/admin.js"></script>
	</head>

	<body>
	</body>
</html>
`
