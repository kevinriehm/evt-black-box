package staevtangel

import (
	"fmt"
	"net/http"
	"strconv"
)

type dataPoint struct {
}

func dataHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" { // Extracting data
		ensureAuthorization(w,r)
		
		start, err := strconv.Atoi(r.FormValue("start"))
		if err != nil {
			http.Error(w,"Bad value for 'start'",http.StatusBadRequest)
			return
		}
		
		end, err := strconv.Atoi(r.FormValue("end"))
		if err != nil {
			http.Error(w,"Bad value for 'end'",http.StatusBadRequest)
			return
		}
		
		fmt.Fprintf(w,"start = %v\nend   = %v",start,end)
	}/* else if r.Method == "POST" { // Submitting data
	}*/
}
