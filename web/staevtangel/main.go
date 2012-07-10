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
		
		<link rel="stylesheet" type="text/css" href="/css/monitor.css">

		<script src="/js/jquery.min.js"></script>
		<script src="/js/jquery.flot.min.js"></script>
		
		<script src="/js/monitor.js"></script>
	</head>

	<body>
		<div id="header">
			<div name="title" class="left">Saint Thomas Academy Experimental Vehicle Team Angel Monitor</div>
			<div name="logout" class="right"><a href="{{.LogoutURL}}">Logout</a></div>
			<div name="user" class="right">{{.Email}}</div>
		</div>

		<div id="panels">
			<!--Monitor panels go here (via Javascipt)-->
			<noscript>This site requires Javascript to be supported and enabled in your browser.</noscript>
		</div>
		
		<div id="logpanel" class="panel">
			<h1>Log</h1>
			<div id="log">
				<!--Messages go here (via Javascript)-->
			</div>
		</div>
	</body>
</html>
`
