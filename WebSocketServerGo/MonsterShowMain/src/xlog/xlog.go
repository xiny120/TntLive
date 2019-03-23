package xlog

import (
	//"fmt"
	//"io"
	"log"
	//"os"
	//"runtime"
	//"sync"
	"time"
)

// Println calls Output to print to the standard logger.
// Arguments are handled in the manner of fmt.Println.
func Println(v ...interface{}) {
	log.Print(time.Now().String())
	log.Println(v...)
}
