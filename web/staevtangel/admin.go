package staevtangel

import (
	"appengine"
	"appengine/datastore"
	
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
		var adminData struct {
			Keys []*datastore.Key
			Exceptions []accessException
		}
		
		// Access exceptions
		adminData.Keys, _ = datastore.NewQuery("accessException").GetAll(c,&adminData.Exceptions)
		
		adminTemplate.Execute(w,&adminData)
	} else if r.Method == "POST" { // AJAX update
		switch(r.FormValue("action")) {
			case "add_exception":
				var response AdminAJAXResponse
				oldKey, newKey := addAccessException(c,r.FormValue("email"),r.FormValue("authorized") == "true")
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
				oldKey := deleteAccessException(c,r.FormValue("key"))
				if oldKey != nil {
					response.Deleted = []string{oldKey.Encode()}
				}
				responseJSON, _ := json.Marshal(&response)
				w.Header().Set("Content-Type","application/json")
				w.Write(responseJSON)
		}
	}
}

var adminTemplate = template.Must(template.New("admin").Parse(adminTemplateString))
var adminTemplateString = `
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
				<input name="email" type="email" required="required">
				
				<label for="authorized">Authorized:</label>
				<input name="authorized" type="checkbox" value="true">
				
				<input type="submit" value="Add Exception">
			</form>
			<ul>
				{{range $i, $key := .Keys}}
					<li class="exception">
						<form>
							<input name="action" type="hidden" value="delete_exception">
							<input name="key" type="hidden" value="{{.Encode}}">
							
							<label class="email_label">{{with index $.Exceptions $i}}{{.Email}}{{end}}</label>
							
							<label class="authorized_label">{{with index $.Exceptions $i}}{{if .Authorized}}authorized{{else}}not authorized{{end}}{{end}}</label>
							
							<input type="submit" value="Delete Exception">
						</form>
					</li>
				{{end}}
			</ul>
		</div>
	</body>
</html>
`
