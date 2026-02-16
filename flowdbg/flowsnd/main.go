package main

import (
	"io"
	"log"
	"net"
	"os"
	"os/exec"
)

type udpSender struct {
	conn   *net.UDPConn
	remote *net.UDPAddr
}

func (u udpSender) Write(p []byte) (n int, err error) {
	u.conn.WriteToUDP(p, u.remote)
	return len(p), nil
}

func (u udpSender) Close() error {
	return u.conn.Close()
}

func newUpdSender(addr string) io.WriteCloser {
	var s udpSender
	var err error
	s.remote, err = net.ResolveUDPAddr("udp", os.Args[1])
	if err != nil {
		log.Fatalf("Connect to %s: %s", os.Args[1], err.Error())
	}

	s.conn, err = net.ListenUDP("udp", nil)
	if err != nil {
		log.Fatalf("ListenUDP error: %s", err.Error())
	}
	return s
}

func main() {
	if len(os.Args) == 1 || os.Args[1] == "-h" || os.Args[1] == "--help" || len(os.Args) != 3 {
		println("Usage: flowsnd IP:PORT RATE")
		os.Exit(0)
	}

	snd := newUpdSender(os.Args[1])

	pv := exec.Command("pv", "-L", os.Args[2], "/dev/zero")
	pv.Stderr = os.Stderr
	stdout, err := pv.StdoutPipe()
	if err != nil {
		log.Fatalf("Can't run pv: %s", err.Error())
	}

	buf := make([]byte, 1200)
	go io.CopyBuffer(snd, stdout, buf)
	pv.Start()
	pv.Wait()
}
