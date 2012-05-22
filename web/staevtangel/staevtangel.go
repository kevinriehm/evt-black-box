package staevtangel

import {
	"fmt"
	"net/http"
}

func init() {
	http.HandleFunc("/",rootHandler)
}

func rootHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprint(w,"Successful test of rootHandler")
}
