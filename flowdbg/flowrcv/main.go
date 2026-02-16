package main

import (
	"golang.org/x/text/language"
	"golang.org/x/text/message"
	"io"
	"net"
	"sync/atomic"
	"time"
)

var bytes uint64

type BpsWriter struct{}

func (b *BpsWriter) Write(p []byte) (n int, err error) {
	atomic.AddUint64(&bytes, uint64(len(p)))
	return len(p), nil
}

func main() {
	addr, err := net.ResolveUDPAddr("udp", ":1234")
	if err != nil {
		panic(err)
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		panic(err)
	}
	defer conn.Close()

	go func() {
		p := message.NewPrinter(language.English)
		for {
			time.Sleep(1 * time.Second)
			p.Printf("%d bps\n", atomic.SwapUint64(&bytes, 0))
		}
	}()
	io.Copy(&BpsWriter{}, conn)
}
