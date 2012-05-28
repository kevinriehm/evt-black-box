package staevtangel

import (
	"appengine"
	"appengine/datastore"
	
	"encoding/json"
	"html/template"
	"net/http"
)

type AdminAJAXResponse struct {
	Deleted string `json:"deleted,omitempty"`
	Added   struct {
		Key        string `json:"key"`
		Email      string `json:"email"`
		Authorized bool   `json:"authorized"`
	} `json:"added,omitempty"`
}

func adminHandler(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)
	
	if r.Method == "GET" { // Regular access
		var adminData struct {
			Keys []*datastore.Key
			Exceptions []AccessException
		}
		
		// Access exceptions
		adminData.Keys, _ = datastore.NewQuery("AccessException").GetAll(c,&adminData.Exceptions)
		
		adminTemplate.Execute(w,&adminData)
	} else if r.Method == "POST" { // AJAX update
		switch(r.FormValue("action")) {
			case "add_exception":
				var response AdminAJAXResponse
				
				email := r.FormValue("email")
				authorized := r.FormValue("authorized") == "true"
				
				oldKeyStr, newKey := addAccessException(c,email,authorized)
				if oldKeyStr != "" {
					response.Deleted = oldKeyStr
				}
				if newKey != nil {
					response.Added.Key = newKey.Encode()
					response.Added.Email = email
					response.Added.Authorized = authorized
				}
				
				responseJSON, _ := json.Marshal(&response)
				w.Header().Set("Content-Type","application/json")
				w.Write(responseJSON)
				
			case "delete_exception":
				var response AdminAJAXResponse
				
				oldKeyStr := deleteAccessException(c,r.FormValue("key"))
				if oldKeyStr != "" {
					response.Deleted = oldKeyStr
				}
				
				responseJSON, _ := json.Marshal(&response)
				w.Header().Set("Content-Type","application/json")
				w.Write(responseJSON)
		}
	}
}

var adminTemplate = template.Must(template.New("admin").Parse(adminTemplateString))
const adminTemplateString = `
<!doctype html>
<html>
	<head>
		<title>Angel Admin Page</title>
		
		<link rel="stylesheet" type="text/css" href="/css/admin.css">
		
		<script src="/js/jquery.min.js"></script>
		<script src="/js/admin.js"></script>
	</head>

	<body>
		<div id="access_exceptions">
			<h1>Access Exceptions</h1>
			
			<form id="add_exception">
				<input name="action" type="hidden" value="add_exception">
				
				<label for="email">Email:</label>
				<input id="email" name="email" type="email" required="required">
				
				<label for="authorized">Authorized:</label>
				<input id="authorized" name="authorized" type="checkbox" value="true">
				
				<input type="submit" value="Add Exception">
			</form>
			
			<ul id="exception_list">
				{{range $i, $key := .Keys}}
					<li>
						<div class="exception_key">{{.Encode}}</div>
						<div class="exception_email">{{with index $.Exceptions $i}}{{.Email}}{{end}}</div>
						<div class="exception_authorized">{{with index $.Exceptions $i}}{{.Authorized}}{{end}}</div>
					</li>
				{{end}}
			</ul>
		</div>
	</body>
</html>
`
