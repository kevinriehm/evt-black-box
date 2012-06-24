package staevtangel

import (
	"appengine"
	"appengine/user"
	
	"html/template"
	"net/http"
)


type MonitorTemplateData struct {
	Email		string
	LogoutURL	string
}


func init() {
	http.HandleFunc("/",monitorHandler)
	http.HandleFunc("/admin",adminHandler)
	http.HandleFunc("/data",dataHandler)
}

func monitorHandler(w http.ResponseWriter, r *http.Request) {
	var templateData MonitorTemplateData
	
	c := appengine.NewContext(r)
	u := user.Current(c)
	
	if !userAuthorized(c,u) {
		redirectToLogin(w,r)
		return
	}
	
	templateData.Email = u.Email
	templateData.LogoutURL, _ = user.LogoutURL(appengine.NewContext(r),r.URL.String())

	monitorTemplate.Execute(w,&templateData)
}

var monitorTemplate = template.Must(template.New("monitor").Parse(monitorTemplateString))
const monitorTemplateString = `
<!doctype html>
<html>
	<head>
		<title>STA EVT Angel</title>

		<script src="/js/jquery.min.js"></script>
		<script src="/js/monitor.js"></script>
	</head>

	<body>
		<div id="header">
			<div name="title">Saint Thomas Academy Experimental Vehicle Team Angel Monitor</div>
			<div name="user">{{.Email}}</div>
			<div name="logout"><a href="{{.LogoutURL}}">Logout</a></div>
		</div>

		<div id="panels">
			<!--Monitor panels go here (via Javascipt)-->
			<noscript>This site requires Javascript to be supported and enabled in your browser.</noscript>
		</div>
		
		<div id="log">
			<!--Messages go here (via Javascript)-->
		</div>
	</body>
</html>
`
