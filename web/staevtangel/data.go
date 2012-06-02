package staevtangel

import (
	"fmt"
	"net/http"
	"time"
)

type Datum struct {
	
}

// Go's standard time, formatted our way
const timeLayout = "20060102150405-0700";

func dataHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" { // Extracting data
		ensureAuthorization(w,r)
		
		start, err := time.Parse(timeLayout,r.FormValue("start"))
		if err != nil {
			fmt.Fprintln(w,"bad value for start")
		}
		
		end, err := time.Parse(timeLayout,r.FormValue("end"))
		if err != nil {
			fmt.Fprintln(w,"bad value for end")
		}
		
		fmt.Fprintf(w,"start = %v\nend   = %v",start.Unix(),end.Unix())
	}/* else if r.Method == "POST" { // Submitting data
	}*/
}

func getDatum(id uint64) Datum {
	return Datum{}
}
