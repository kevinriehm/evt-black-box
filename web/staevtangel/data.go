package staevtangel

import (
	"appengine"
	"appengine/datastore"
	"appengine/memcache"
	"appengine/user"

	"crypto/hmac"
	"crypto/sha256"
	"crypto/subtle"
	
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strconv"
	"strings"
)


var hmacKey = []byte{
		0xF5,0x2B,0x58,0x46,0x1A,0x02,0xC9,0xFE,
		0xF8,0xA6,0x6F,0xD3,0xE0,0xC8,0x9C,0xB7,
		0xDA,0x42,0x2C,0x38,0xC0,0xCA,0xD1,0x9A,
		0x94,0x47,0x6F,0x74,0x98,0x63,0x99,0xB3,
	}


type Datum struct {
	Time          int64 `json:"time"`
	Potentiometer int   `json:"potentiometer"`
}

func GetDatum(c appengine.Context, car string, id int64) (*Datum, error) {
	var d Datum
	
	strID := car + strconv.FormatInt(id,10)
	
	if _, err := memcache.Gob.Get(c,strID,&d); err != nil {
		// Not in the cache, presumably
		if err = datastore.Get(c,datastore.NewKey(c,car + " Datum","",id,nil),&d); err != nil {
			return nil, err
		}
		
		// Speed this up later
		memcache.Gob.Set(c,&memcache.Item{Key: strID, Object: &d})
	}
	
	return &d, nil
}

func GetData(c appengine.Context, car string, start, end int64) ([]*Datum, error) {
	var data []*Datum
	
	q := datastore.
		NewQuery(car + " Datum").
		KeysOnly().
		Filter("Time >=",start).
		Filter("Time <=",end)
	
	if k, err := q.Order("Time").Run(c).Next(nil); err == nil {
		start = k.IntID()
	} else {
		return nil, err
	}
	
	if k, err := q.Order("-Time").Run(c).Next(nil); err == nil {
		end = k.IntID()
	} else {
		return nil, err
	}
	
	if n, err := q.Count(c); err == nil {
		data = make([]*Datum,n)
	} else {
		return nil, err
	}
	
	for i := 0; start <= end; start++ {
		if d, err := GetDatum(c,car,start); err == nil {
			data[i] = d
			i++
		} else {
			return nil, err
		}
	}
	
	return data, nil
}

func (d Datum) Put(c appengine.Context, car string) {
	datastore.Put(c,datastore.NewKey(c,car + " Datum","",d.Time,nil),&d)
	memcache.Gob.Set(c,&memcache.Item{Key: strconv.FormatInt(d.Time,10), Object: &d})
	
	tempDatum := &Datum{}
	strID := car + " Last Datum"
	memcache.Gob.Get(c,strID,tempDatum)
	if d.Time > tempDatum.Time {
		memcache.Gob.Set(c,&memcache.Item{Key: strID, Object: &d})
	}
}

func (d Datum) PrintHeader(w http.ResponseWriter) {
	w.Write([]byte("Unix Time,Potentiometer\x0D\x0A"))
}

func (d Datum) String() string {
	return fmt.Sprint(d.Time,",",d.Potentiometer)
}


type TeedReadCloser struct {
	io.ReadCloser
	
	rc io.ReadCloser
	w  io.Writer
}

func teeReadCloser(rc io.ReadCloser, w io.Writer) io.ReadCloser {
	return TeedReadCloser{rc: rc, w: w}
}

func (trc TeedReadCloser) Read(p []byte) (n int, err error) {
	n, err = trc.rc.Read(p)
	trc.w.Write(p[:n])
	return
}

func (trc TeedReadCloser) Close() error {
	return trc.rc.Close()
}


func dataHandler(w http.ResponseWriter, r *http.Request) {
	c := appengine.NewContext(r)

	if r.Method == "GET" { // Extracting data
		if !userAuthorized(c,user.Current(c)) {
			redirectToLogin(w,r)
			return
		}
		
		switch r.FormValue("type") {
			case "ajax":
				var datum *Datum
				
				car := strings.ToLower(r.FormValue("car"))
				if !validCar(car) {
					http.Error(w,"bad value for car",http.StatusBadRequest)
					return
				}
				
				// An empty time asks for the most recent entry
				if r.FormValue("time") == "" {
					datum = &Datum{}
					strID := car + " Last Datum"
					if _, err := memcache.Gob.Get(c,strID,datum); err != nil {
						if _, err := datastore.NewQuery(car + " Datum").Order("-Time").Run(c).Next(datum); err != nil && err != datastore.Done {
							http.Error(w,"no data",http.StatusInternalServerError)
							return
						}
						
						memcache.Gob.Set(c,&memcache.Item{Key: strID, Object: &datum})
					}
				} else { // Otherwise be specific
					datumTime, err := strconv.ParseInt(r.FormValue("time"),0,64)
					if err != nil && datumTime != 0 {
						http.Error(w,"bad value for time",http.StatusBadRequest)
						return
					}
					
					if datum, err = GetDatum(c,car,datumTime); err != nil {
						http.Error(w,"cannot get datum " + strconv.FormatInt(datumTime,10),http.StatusInternalServerError)
						return
					}
				}
				
				w.Header().Set("Content-Type","application/json")
				responseJSON, _ := json.Marshal(datum)
				w.Write(responseJSON)
				
			case "csv":
				// Parse the arguments
	
				car := strings.ToLower(r.FormValue("car"))
				if !validCar(car) {
					http.Error(w,"bad value for car",http.StatusBadRequest)
					return
				}
	
				start, err := strconv.ParseInt(r.FormValue("start"),0,64)
				if err != nil {
					http.Error(w,"bad value for start",http.StatusBadRequest)
					return
				}
				
				end, err := strconv.ParseInt(r.FormValue("end"),0,64)
				if err != nil {
					http.Error(w,"bad value for end",http.StatusBadRequest)
					return
				}
				
				// Respond
				
				w.Header().Set("Content-Type","text/csv; header=present")
				w.Header().Set("Content-Disposition","attachment; filename=\"" + car + "_" + r.FormValue("start") + "_" + r.FormValue("end") + ".csv\"")
				Datum{}.PrintHeader(w)
				
				data, err := GetData(c,car,start,end)
				if err != nil {
					http.Error(w,fmt.Sprint(err),http.StatusInternalServerError)
					return
				}
				
				for i := 0; i < len(data); i++ {
					fmt.Fprintln(w,data[i])
				}
				
			default:
				http.Error(w,"bad value for type",http.StatusBadRequest)
		}
	} else if r.Method == "POST" { // Inserting data
		var d Datum
		var err interface{}
		
		// Set a wrapper around r.Body to allow authorization checking
		hmac := hmac.New(sha256.New,hmacKey)
		r.Body = teeReadCloser(r.Body,hmac)
		r.ParseForm()
		
		// Check authorization
		b := make([]byte,64)
		fmt.Sscanf(r.Header["Authorization"][0],"%x",&b)
		if subtle.ConstantTimeCompare(b,hmac.Sum([]byte{})) != 1 {
			http.Error(w,"unauthorized POST",http.StatusForbidden)
			return
		}
		
		// Process the arguments
		
		car := strings.ToLower(r.FormValue("car"))
		if !validCar(car) {
			http.Error(w,"bad value for car",http.StatusBadRequest)
			return
		}
				
		d.Time, err = strconv.ParseInt(r.FormValue("time"),0,64)
		if err != nil {
			http.Error(w,"bad value for time",http.StatusBadRequest)
			return
		}
		
		d.Potentiometer, err = strconv.Atoi(r.FormValue("potentiometer"))
		if err != nil {
			http.Error(w,"bad value for potentiometer",http.StatusBadRequest)
			return
		}
		
		d.Put(c,car)
	}
}

func validCar(car string) bool {
	return car == "alpha"
}
